#pragma once

#include <vector>
#include <cmath>

struct Color
{
	float r, g, b;
	Color(float r, float g, float b)
		:r(r), g(g), b(b)
	{}
};

class ColourGenerator 
{
private:
	const float m_spread;
	const float m_halfSpread;
	const int m_vertexCount;

	const std::vector<Color> m_biomeColours;
	const float m_part;

public:
	ColourGenerator(const std::vector<Color>& biomeColours, float spread, int vertexCount)

		:m_biomeColours(biomeColours), m_spread(spread),
		m_halfSpread(spread / 2.f), m_part(1.f / (biomeColours.size() - 1)),
		m_vertexCount(vertexCount)
	{}

	void generateColours(float** heights, float amplitude, float* colors) const
	{
		int pointer = 0;
		for (int z = 0; z < m_vertexCount; z++)
		{
			for (int x = 0; x < m_vertexCount; x++)
			{
				Color c = calculateColour(heights[x][z], amplitude);
				colors[pointer++] = c.r;
				colors[pointer++] = c.g;
				colors[pointer++] = c.b;
			}
		}
	}

private:
	Color interpolateColours(Color a, Color b, float blend) const
	{
		float colorWeight1 = 1.0f - blend;
		float r1 = a.r / 255.0f;
		float g1 = a.g / 255.0f;
		float b1 = a.b / 255.0f;

		float r2 = b.r / 255.0f;
		float g2 = b.g / 255.0f;
		float b2 = b.b / 255.0f;

		float r3 = (colorWeight1 * r1) + (blend * r2);
		float g3 = (colorWeight1 * g1) + (blend * g2);
		float b3 = (colorWeight1 * b1) + (blend * b2);

		return Color(r3,g3,b3);
	}
	Color calculateColour(float height, float amplitude) const
	{
		
		float value = (height + amplitude) / (amplitude * 2);

		// clamping value 
		float v = (value - m_halfSpread) * (1.f / m_spread);
		value = v < 0.0f ? 0.0f : v > 0.9999f ? 0.9999f : v;

		int firstBiome = (int)floor(value / m_part);
		float blend = (value - (firstBiome * m_part)) / m_part;

		return interpolateColours(m_biomeColours[firstBiome], m_biomeColours[firstBiome + 1], blend);
	}
};