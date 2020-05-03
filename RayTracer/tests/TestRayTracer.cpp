#include "TestRayTracer.hpp"

// Available gtest framework macros
// 		EXPECT_TRUE
// 		EXPECT_FALSE
// 		EXPECT_EQ
// 		EXPECT_STREQ
//		EXPECT_NO_THROW
//		EXPECT_ANY_THROW
//		EXPECT_THROW
//		EXPECT_DOUBLE_EQ
//		EXPECT_FLOAT_EQ

TEST(RayTracer, testPass)
{
	EXPECT_EQ(1, 1);
}

TEST(RayTracer, testFail)
{
	EXPECT_EQ(1, 0);
}