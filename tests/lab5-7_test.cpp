#include <gtest/gtest.h>
#include "tsearch.h"
#include "ttopology.h"
#include "tmessaging.h"
#include <atomic>
#include <string>
#include <unistd.h>
#include <pthread.h>

TEST(TSearchTest, EmptyPattern) {
    std::string text = "abracadabra";
    std::string pattern = "";
    auto positions = TSearch::BoyerMooreSearch(text, pattern);
    // Пустой паттерн – вхождение в каждую позицию
    ASSERT_EQ((int)positions.size(), (int)text.size());
    for (int i = 0; i < (int)text.size(); i++) {
        EXPECT_EQ(positions[i], i);
    }
}

TEST(TSearchTest, NotFound) {
    std::string text = "abracadabra";
    std::string pattern = "zzz";
    auto positions = TSearch::BoyerMooreSearch(text, pattern);
    ASSERT_EQ((int)positions.size(), 1);
    EXPECT_EQ(positions[0], -1);
}

TEST(TSearchTest, SimpleFound) {
    std::string text = "abracadabra";
    std::string pattern = "abra";
    auto positions = TSearch::BoyerMooreSearch(text, pattern);
    // Ожидаем вхождения: в позициях 0 и 7
    ASSERT_EQ((int)positions.size(), 2);
    EXPECT_EQ(positions[0], 0);
    EXPECT_EQ(positions[1], 7);
}

TEST(TSearchTest, MultipleOverlapFound) {
    std::string text = "aaaaa";
    std::string pattern = "aa";
    auto positions = TSearch::BoyerMooreSearch(text, pattern);
    // Вхождения: 0,1,2,3
    ASSERT_EQ((int)positions.size(), 4);
    EXPECT_EQ(positions[0], 0);
    EXPECT_EQ(positions[1], 1);
    EXPECT_EQ(positions[2], 2);
    EXPECT_EQ(positions[3], 3);
}


TEST(TTopologyTest, AddNode) {
    TTopology topo;
    EXPECT_FALSE(topo.NodeExists(10));
    EXPECT_TRUE(topo.AddNode(10, -1, "endpoint10"));
    EXPECT_TRUE(topo.NodeExists(10));

    TNodeInfo* info = topo.GetNode(10);
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->id, 10);
    EXPECT_EQ(info->parent_id, -1);
    EXPECT_EQ(info->endpoint, "endpoint10");
    EXPECT_TRUE(info->alive);
}

TEST(TTopologyTest, AddDuplicateNode) {
    TTopology topo;
    EXPECT_TRUE(topo.AddNode(10, -1, "endpoint10"));
    EXPECT_FALSE(topo.AddNode(10, -1, "endpoint10_bis")); // Уже существует
}

TEST(TTopologyTest, RemoveNode) {
    TTopology topo;
    topo.AddNode(10, -1, "end10");
    topo.AddNode(20, 10, "end20");
    EXPECT_TRUE(topo.NodeExists(20));
    EXPECT_TRUE(topo.RemoveNode(20));
    EXPECT_FALSE(topo.NodeExists(20));
}

TEST(TTopologyTest, GetChildren) {
    TTopology topo;
    topo.AddNode(10, -1, "end10");
    topo.AddNode(20, 10, "end20");
    topo.AddNode(15, 10, "end15");
    topo.AddNode(12, -1, "end12");

    auto children10 = topo.GetChildren(10);
    ASSERT_EQ((int)children10.size(), 2);
    EXPECT_TRUE(std::find(children10.begin(), children10.end(), 20) != children10.end());
    EXPECT_TRUE(std::find(children10.begin(), children10.end(), 15) != children10.end());

    auto childrenRoot = topo.GetChildren(-1);
    ASSERT_EQ((int)childrenRoot.size(), 2); // 10 и 12
    EXPECT_TRUE(std::find(childrenRoot.begin(), childrenRoot.end(), 10) != childrenRoot.end());
    EXPECT_TRUE(std::find(childrenRoot.begin(), childrenRoot.end(), 12) != childrenRoot.end());
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
