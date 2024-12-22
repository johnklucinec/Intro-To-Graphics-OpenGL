#ifndef BLOCKS_H
#define BLOCKS_H

#include "glslprogram.h"
#include <vector>

struct Block
{
	glm::vec3 position;
	glm::vec4 color;
	bool active = false;

	bool operator==(const Block& other) const
	{
		return position == other.position &&
			color == other.color &&
			active == other.active;
	}
};

struct InstanceData
{
	glm::vec3 position;
	glm::vec4 color;
};



extern std::vector<std::vector<Block>> levels;

#endif // BLOCKS_H
