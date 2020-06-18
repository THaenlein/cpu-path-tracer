#include "TestBoundingBox.hpp"

#include "../src/Types/BoundingBox.hpp"

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

TEST(BoundingBox, TestIntersects)
{
	aiVector3D min(1.f, 1.f, 0.f);
	aiVector3D max(2.f, 2.f, 1.f);
	raytracing::BoundingBox box(min, max);

	aiVector3D position(0.f, 0.f, .5f);
	aiVector3D direction(1.f, 1.f, 0.f);
	aiRay ray(position, direction);

	bool intersects = box.intersects(ray);
	EXPECT_TRUE(intersects);
}

TEST(BoundingBox, TestNotIntersects)
{
	aiVector3D min(1.f, 1.f, 0.f);
	aiVector3D max(2.f, 2.f, 1.f);
	raytracing::BoundingBox box(min, max);

	aiVector3D position(0.f, 0.f, .5f);
	aiVector3D direction(0.f, 0.f, 0.f);
	aiRay ray(position, direction);

	bool doesNotIntersect = !box.intersects(ray);
	EXPECT_TRUE(doesNotIntersect);
}
