class IndexGenerator
{
public:
	int storeQuad1(unsigned int* indices, int pointer, int topLeft, int topRight, int bottomLeft, int bottomRight,
		bool mixed) {
		indices[pointer++] = topLeft;
		indices[pointer++] = bottomLeft;
		indices[pointer++] = mixed ? topRight : bottomRight;
		indices[pointer++] = bottomRight;
		indices[pointer++] = topRight;
		indices[pointer++] = mixed ? bottomLeft : topLeft;
		return pointer;
	}

	int storeQuad2(unsigned int* indices, int pointer, int topLeft, int topRight, int bottomLeft, int bottomRight,
		bool mixed) {
		indices[pointer++] = topRight;
		indices[pointer++] = topLeft;
		indices[pointer++] = mixed ? bottomRight : bottomLeft;
		indices[pointer++] = bottomLeft;
		indices[pointer++] = bottomRight;
		indices[pointer++] = mixed ? topLeft : topRight;
		return pointer;
	}
};