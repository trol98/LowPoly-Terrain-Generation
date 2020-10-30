

class NormalGenerator
{
	const int m_vertexCount;
public:

	NormalGenerator(int vertex_count)
		:m_vertexCount(vertex_count)
	{}

	void generateNormals(float** heights, float* normals) const
	{
		int pointer = 0;
		for (int z = 0; z < m_vertexCount; z++) {
			for (int x = 0; x < m_vertexCount; x++) {
				glm::vec3 norm = calculateNormal(x, z, heights);
				normals[pointer++] = norm.x;
				normals[pointer++] = norm.y;
				normals[pointer++] = norm.z;
			}
		}
	}
private:

	float getHeight(int x, int z, float** heights) const
	{
		x = x < 0 ? 0 : x;
		z = z < 0 ? 0 : z;
		x = x >= m_vertexCount ? m_vertexCount - 1 : x;
		z = z >= m_vertexCount ? m_vertexCount - 1 : z;
		return heights[z][x];
	}

	glm::vec3 calculateNormal(int x, int z, float** heights) const
	{
		float heightL = getHeight(x - 1, z, heights);
		float heightR = getHeight(x + 1, z, heights);
		float heightD = getHeight(x, z - 1, heights);
		float heightU = getHeight(x, z + 1, heights);
		glm::vec3 normal = glm::vec3(heightL - heightR, 2.f, heightD - heightU);
		normal = glm::normalize(normal);
		return normal;
	}
};

