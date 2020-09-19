#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

struct Color
{
	float r, g, b;
	Color(float _r, float _g, float _b)
		:r(_r), g(_g), b(_b)
	{}
};

class ColourGenerator {
private:
	float spread;
	float halfSpread;

	std::vector<Color> biomeColours;
	float part;

	/**
	 * @param biomeColours
	 *            - The preset colours that will be interpolated over the
	 *            terrain. The first colours in this array will be used for the
	 *            lowest parts of the terrain, and the last colours in this
	 *            array will be used for the highest. All the other colours will
	 *            be spread out linearly inbetween.
	 * @param spread
	 *            - This indicates how much of the possible altitude range the
	 *            colours should be spread over. If this is too high the extreme
	 *            colours won't be used as there won't be any terrain vertices
	 *            high or low enough (the heights generator doesn't usually fill
	 *            the whole altitude range).
	 */
public:
	ColourGenerator(const std::vector<Color>& biomeColours, float spread)
		:biomeColours(biomeColours), spread(spread), halfSpread(spread / 2.f), part(1.f / (biomeColours.size() - 1))
	{}

	/**
	 * Calculates the colour for every vertex of the terrain, by linearly
	 * interpolating between the biome colours depending on the vertex's height.
	 *
	 * @param heights
	 *            -The heights of all the vertices in the terrain.
	 * @param amplitude
	 *            - The amplitude range of the terrain that was used in the
	 *            heights generation. Maximum possible height is
	 *            {@code altitude} and minimum possible is {@code -altitude}.
	 * @return The colours of all the vertices in the terrain, in a grid.
	 */
	void generateColours(float** heights, float amplitude, float* colors, int size)
	{
		int pointer = 0;
		for (int z = 0; z < size; z++)
		{
			for (int x = 0; x < size; x++)
			{
				Color c = calculateColour(heights[x][z], amplitude);
				colors[pointer++] = c.r;
				colors[pointer++] = c.g;
				colors[pointer++] = c.b;
			}
		}
	}

	/**Determines the colour of the vertex based on the provided height.
	 * @param height - Height of the vertex.
	 * @param amplitude - The maximum height that a vertex can be (
	 * @return
	 */
private:
	Color interpolateColours(Color a, Color b, float blend)
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
	Color calculateColour(float height, float amplitude) {
		
		float value = (height + amplitude) / (amplitude * 2);

		// clamping value 
		float v = (value - halfSpread) * (1.f / spread);
		value = v < 0.0f ? 0.0f : v > 0.9999f ? 0.9999f : v;

		int firstBiome = (int)floor(value / part);
		float blend = (value - (firstBiome * part)) / part;
		return interpolateColours(biomeColours[firstBiome], biomeColours[firstBiome + 1], blend);

		
	}
};