#include <gtest/gtest.h>

#include "assimp\Importer.hpp"
#include "assimp\postprocess.h"
#include "assimp\scene.h"

struct TestBoundingBox : public testing::Test
{
	virtual void SetUp() override
	{

	}

	virtual void TearDown() override
	{

	}
	
};
	
static const aiScene* importAndProcess(Assimp::Importer& importer, const std::string& filePath);
