#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <unistd.h>
using uint = unsigned int;
struct State
{
	const uint rows;
	const uint cols;
	bool* data;
	State()= default;
	State(uint rows, uint cols): rows(rows), cols(cols)
	{
		data = new bool[rows*cols];
	}
	State(const State& rhs):
		rows(rhs.rows), cols(rhs.cols)
	{
		data = new bool[rows*cols*sizeof(bool)];
		memcpy(data, rhs.data, rows*cols*sizeof(bool));
	}
	State(State&& rhs):
		rows(rhs.rows), cols(rhs.cols)
	{
		data = rhs.data;
		rhs.data = nullptr;
	}
	~State(){delete[] data;}

	inline bool& operator()(uint row, uint col){ return data[row*cols + col];}
	inline const bool& operator()(uint row, uint col) const { return data[row*cols + col];}

	inline bool& at(uint row, uint col){ return data[row*cols + col];}
	inline const bool& at(uint row, uint col) const { return data[row*cols + col];}

	inline uint aliveNeightbours(uint row, uint col) const
	{
		uint neightbours = 0;
		for(int i = -1; i <= 1; ++i)
		{
			for(int j = -1; j <= 1; ++j)
			{
				uint index =
							(row + rows + i) % rows * cols +
							(col + cols + j) % cols;
				neightbours += data[index];
			}
		}
		return neightbours;
	}
	inline void print() const
	{
		auto separator = [cols = this->cols](){
			for(uint i = 0; i < cols + 2; i++)
				printf("=");
			printf("\n");
		};
		printf("\033[2J");
		
		separator();
		for(uint i = 0; i < rows; i++)
		{
			printf("|");
			for(uint j = 0; j < cols; j++)
			{
				printf("%c", data[i * cols + j]? '+':'-');
			}
			printf("|\n");
		}
	}
};

//1. Any live cell with fewer than two live neighbours dies, as if by underpopulation.
//2. Any live cell with two or three live neighbours lives on to the next generation.
//3. Any live cell with more than three live neighbours dies, as if by overpopulation.
//4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
bool apply_rules(bool alive, uint neightbours)
{
	return alive
		?	neightbours == 2 || neightbours == 3
		:	neightbours == 3;
}

int main(int argc, char const *argv[])
{
	uint rows = 10, cols= 10;
	if(argc == 3)
	{
		rows = atoi(argv[1]);
		cols = atoi(argv[2]);
	}

	State state0(rows, cols);
	State state1(rows, cols);
	State*		state = &state0;
	State* next_state = &state1;

	for(uint i = 0; i < rows; ++i)
		for (uint j = 0; j < cols; j++)
			state->at(i, j) = bool(rand() & 1);
	uint gen = 0;
	while (1)
	{
		printf("Generation %u\n", gen++);
		for(uint i = 0; i < rows; ++i)
		{
			for(uint j = 0; j < cols; ++j)
			{
				next_state->at(i, j) = apply_rules(state->at(i, j), state->aliveNeightbours(i, j));
			}
		}
		state->print();
		sleep(3);
		std::swap(state, next_state);
	}
	return 0;
}
