#include "TestBoundingBox.hpp"

#include "..\src\Types\BoundingBox.hpp"

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


// Regular intersection
TEST(BoundingBox, TestIntersectsRegular)
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

// Negative direction vector
TEST(BoundingBox, TestIntersectsNegative)
{
	aiVector3D min(1.f, 1.f, 0.f);
	aiVector3D max(2.f, 2.f, 1.f);
	raytracing::BoundingBox box(min, max);

	aiVector3D position(3.f, 3.f, .5f);
	aiVector3D direction(-1.f, -1.f, 0.f);
	aiRay ray(position, direction);

	bool intersects = box.intersects(ray);
	EXPECT_TRUE(intersects);
}

// Intersects in both directions
TEST(BoundingBox, TestIntersectsBoth)
{
	aiVector3D min(-1.f, -1.f, 0.f);
	aiVector3D max(1.f, 1.f, 1.f);
	raytracing::BoundingBox box(min, max);

	aiVector3D position(-2.f, 0.f, .5f);
	aiVector3D direction(-1.f, 0.f, 0.f);
	aiRay ray(position, direction);

	bool intersects = box.intersects(ray);
	EXPECT_TRUE(intersects);
}

// Line intersection
TEST(BoundingBox, TestIntersectsLine)
{
	aiVector3D min(0.f, 0.f, 0.f);
	aiVector3D max(1.f, 1.f, 1.f);
	raytracing::BoundingBox box(min, max);

	aiVector3D position(-1.f, 0.f, .5f);
	aiVector3D direction(1.f, 0.f, 0.f);
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
	aiVector3D direction(0.f, 0.f, 1.f);
	aiRay ray(position, direction);

	bool doesNotIntersect = !box.intersects(ray);
	EXPECT_TRUE(doesNotIntersect);
}

TEST(BoundingBox, TestNotIntersectsNegative)
{
	aiVector3D min(1.f, 1.f, 0.f);
	aiVector3D max(2.f, 2.f, 1.f);
	raytracing::BoundingBox box(min, max);

	aiVector3D position(3.f, 3.f, .5f);
	aiVector3D direction(-1.f, 0.f, 0.f);
	aiRay ray(position, direction);

	bool doesNotIntersect = !box.intersects(ray);
	EXPECT_TRUE(doesNotIntersect);
}


TEST(BoundingBox, TestSplitX)
{
	aiVector3D min(0.f, 0.f, 0.f);
	aiVector3D max(2.f, 1.f, 1.f);
	raytracing::BoundingBox box(min, max);

	raytracing::BoundingBox left;
	raytracing::BoundingBox right;

	box.split(left, right);

	raytracing::BoundingBox expectedLeft(min, aiVector3D(1.f, 1.f, 1.f));
	EXPECT_EQ(left, expectedLeft);

	raytracing::BoundingBox expectedRight(aiVector3D(1.f, 0.f, 0.f), max);
	EXPECT_EQ(right, expectedRight);
}

TEST(BoundingBox, TestSplitY)
{
	aiVector3D min(0.f, 0.f, 0.f);
	aiVector3D max(1.f, 3.f, 1.f);
	raytracing::BoundingBox box(min, max);

	raytracing::BoundingBox left;
	raytracing::BoundingBox right;

	box.split(left, right);

	raytracing::BoundingBox expectedLeft(min, aiVector3D(1.f, 1.5f, 1.f));
	EXPECT_EQ(left, expectedLeft);

	raytracing::BoundingBox expectedRight(aiVector3D(0.f, 1.5f, 0.f), max);
	EXPECT_EQ(right, expectedRight);
}

TEST(BoundingBox, TestSplitZ)
{
	aiVector3D min(0.f, 0.f, 0.f);
	aiVector3D max(1.f, 1.f, 4.f);
	raytracing::BoundingBox box(min, max);

	raytracing::BoundingBox left;
	raytracing::BoundingBox right;

	box.split(left, right);

	raytracing::BoundingBox expectedLeft(min, aiVector3D(1.f, 1.f, 2.f));
	EXPECT_EQ(left, expectedLeft);

	raytracing::BoundingBox expectedRight(aiVector3D(0.f, 0.f, 2.f), max);
	EXPECT_EQ(right, expectedRight);
}