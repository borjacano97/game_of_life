#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <unistd.h>
#include <SDL2/SDL.h>

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
	inline static void swap(State& a, State& b)
	{
		auto aux = a.data;
		a.data = b.data;
		b.data = aux;
	}
	inline uint aliveNeightbours(uint row, uint col) const
	{
		// uint neightbours = 0;
		// for(int i = -1; i <= 1; ++i)
		// {
		// 	for(int j = -1; j <= 1; ++j)
		// 	{
		// 		uint index =
		// 					(row + rows + i) % rows * cols +
		// 					(col + cols + j) % cols;
		// 		neightbours += data[index];
		// 	}
		// }
		// return neightbours;
		uint n = 0;
		int r, c;
		for(int i = -1; i <=1; i++)
		{
			for(int j = -1; j <= 1; j++)
			{
				r = row + i;
				c = col + j;
				r = r < 0 ? rows - 1 : r == rows - 1 ? 0 : r;
				c = c < 0 ? cols - 1 : c == cols - 1 ? 0 : c;
				n += at(r, c);
			}
		}
		return n;
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
struct Window
{
	uint width, height, pixel_size;
	SDL_Window* window;
	SDL_Renderer* renderer;
	static bool initialized;
	Window(uint width, uint height, uint pixel_size):
		width(width), height(height), pixel_size(pixel_size)
	{
		if(!initialized)
		{
			initialized = true;
			SDL_Init(SDL_INIT_VIDEO);
		}
		SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);
	}
	~Window()
	{
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
	}
	void PrintState(const State& state)
	{
		SDL_SetRenderDrawColor(renderer, 0,0,0,255);
		SDL_RenderClear(renderer);
		for(uint row = 0; row < state.rows; ++row)
		{
			for(uint col=0; col < state.cols; ++col)
			{
				state.at(row, col) ?
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255):
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				// float r = row / float(state.rows);
				// float g = col / float(state.cols);
				// float b = 0.5f;

				// SDL_SetRenderDrawColor(renderer, Uint8(r*255), Uint8(g*255), Uint8(b*255), Uint8(255));
				SDL_Rect pixel;
				pixel.x = col * pixel_size;
				pixel.y = row * pixel_size;
				pixel.h = pixel_size;
				pixel.w = pixel_size;
				//SDL_FillRect(renderer, &pixel);
				SDL_RenderFillRect(renderer, &pixel);
			}
		}
		SDL_RenderPresent(renderer);
	}

};
bool Window::initialized = false;
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
void random_populate(State& state)
{
	for(uint i = 0; i< state.rows; i++)
		for(uint j = 0; j < state.cols; j++)
			state.at(i, j) = rand() & 1;
}
void handle_events(bool& close, State& state)
{
	SDL_Event e;
	while(SDL_PollEvent(&e))
	{
		if(e.type == SDL_QUIT)
		{
			SDL_Quit();
			close = true;
		}
		else if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE)
		{
			random_populate(state);
		}
	}
}
void sim_step(State& state, State& next_state)
{
	for(uint i = 0; i < state.rows; ++i)
	{
		for(uint j = 0; j < state.cols; ++j)
		{
			next_state(i, j) = apply_rules(state(i, j), state.aliveNeightbours(i, j));
		}
	}
}
int main(int argc, char const *argv[])
{
	uint rows = 100, cols= 100;
	if(argc == 3)
	{
		rows = atoi(argv[1]);
		cols = atoi(argv[2]);
	}

	State state(rows, cols);
	State next_state(rows, cols);
	random_populate(state);
	uint gen = 0;
	bool close = false;
	uint win_width = 500;
	uint win_height = 500;
	uint pixel_size = win_width / cols;
	Window window(win_width, win_height, pixel_size);
	while (!close)
	{
		handle_events(close, state);
		sim_step(state, next_state);
		window.PrintState(state);
		printf("Generation: %d\r", gen++);
		SDL_Delay(10000);
		
		//swap states
		State::swap(state, next_state);
	}
	return 0;
}
