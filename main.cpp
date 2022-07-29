#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <algorithm>
#include <set>
#include <utility>

const double SPEED_NORMAL = 0.5;
const double SPEED_QUICK = 0.05;
const double DECLINE = 0.9;

struct Assets
{
	sf::Sprite board;
	sf::Sprite tiles;
	sf::Text text;
};

class Board
{
	private:

		int **board;
		int board_row;
		int board_col;

		int tetro_x[4];
		int tetro_y[4];
		int tetro_type;

		int score;
		int lines;

		double tetro_fac[7];

	public:

		Board(int row, int col, int tetro, int** begin_board = NULL);
		~Board();

		unsigned long long hashBoard();

		int evaluateBoard();

		void drawBoard(sf::RenderWindow *window, Assets *assets);

		int checkLine();

		double searchMax(int tet, std::set<unsigned long long> &set);

		int selectTetro();

		void initTetro(int tet);

		void rotateTetro(int dx);

		void moveTetro(int dx);

		bool dropTetro();

		void harddropTetro();
};

int main()
{
	srand(time(0));

	sf::RenderWindow window(sf::VideoMode(576, 768), "Bastet!");

	Board board(20, 10, rand() % 7 + 1);

	sf::Clock clock;
	Assets assets;

	sf::Texture tex_board;
	tex_board.loadFromFile("background.png");
	assets.board = sf::Sprite(tex_board);

	sf::Texture tex_tiles;
	tex_tiles.loadFromFile("tiles.png");
	assets.tiles = sf::Sprite(tex_tiles);

	sf::Font font;
	font.loadFromFile("dogicapixelbold.ttf");
	assets.text = sf::Text("", font, 18);

	double timer = 0;
	double delay = SPEED_NORMAL;

	while (window.isOpen())
	{
		float time = clock.getElapsedTime().asSeconds();
		timer += time;
		clock.restart();

		sf::Event e;
		bool rotate = false;
		int dx = 0;
		int dy = 0;
		bool harddrop = false;

		while (window.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
			{
				window.close();
			}

			if (e.type == sf::Event::KeyPressed)
			{
				switch (e.key.code)
				{
				case sf::Keyboard::Up:

					dy = -1;
					break;

				case sf::Keyboard::Space:

					dy = 1;
					break;

				case sf::Keyboard::Left:

					dx = -1;
					break;

				case sf::Keyboard::Right:

					dx = 1;
					break;

				case sf::Keyboard::Enter:

					harddrop = true;
					break;
				}
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			{
				delay = SPEED_QUICK;
			}

			if (dx)
			{
				board.moveTetro(dx);
			}

			if (dy)
			{
				board.rotateTetro(dy);
			}

			if (harddrop)
			{
				board.harddropTetro();
				board.initTetro(board.selectTetro());
			}
		}
		if (timer > delay)
		{
			if (board.dropTetro())
			{
				board.initTetro(board.selectTetro());
			}
			timer = 0;
		}
		board.checkLine();

		delay = SPEED_NORMAL;

		board.drawBoard(&window, &assets);

		window.display(); 
	}

	return 0;
}

Board::Board(int row, int col, int tetro, int** begin_board)
{
	board = new int*[row];

	for (int i = 0; i < row; i++)
	{
		board[i] = new int[col]();
	}

	board_row = row;
	board_col = col;

	score = lines = 0;

	this->initTetro(tetro);

	for (int i = 0; i < 7; i++)
	{
		tetro_fac[i] = 0.5 + 0.5 * rand() / RAND_MAX;
	}

	if (begin_board)
	{
		for (int i = 0; i < row; i++)
		{
			memcpy(board[i], begin_board[i], sizeof(int) * col);
		}
	}
}

Board::~Board()
{
	for (int i = 0; i < board_row; i++)
	{
		delete[] board[i];
	}
	delete[] board;
}

unsigned long long Board::hashBoard()
{
	unsigned long long hash = 0;
	for (int i = 0; i < 4; i++)
	{
		if (tetro_y[i] < 0)
		{
			continue;
		}
		board[tetro_y[i]][tetro_x[i]] = 8;
	}
	for (int i = 0; i < board_row; i++)
	{
		for (int j = 0; j < board_col; j++)
		{
			hash = hash * 11 + board[i][j];
		}
	}
	for (int i = 0; i < 4; i++)
	{
		if (tetro_y[i] < 0)
		{
			continue;
		}
		board[tetro_y[i]][tetro_x[i]] = 0;
	}
	return hash;
}

int Board::evaluateBoard()
{
	int ret = 0, sum = 500, min = board_row;

	for (int i = board_row - 1; i >= 0; i--)
    {
        int count = 0;
        for (int j = 0; j < board_col; j++)
        {
            if (board[i][j])
			{
                count++;
			}
        }
        if (count == board_col)
		{
			ret += sum;
			sum *= 5;
		}
    }
	for (int i = 0; i < board_col; i++)
	{
		int cnt = 0;
		while (cnt < board_row && !board[cnt][i])
		{
			cnt++;
		}
		ret += cnt;
		min = std::min(min, cnt);
	}
	ret += min;
	return ret;
}

void Board::drawBoard(sf::RenderWindow *window, Assets *assets)
{
	window->draw(assets->board);

	for (int i = 0; i < board_row; i++)
	{
		for (int j = 0; j < board_col; j++)
		{
			if (!board[i][j]) 
			{
				continue; 
			}
			sf::IntRect rect = sf::IntRect(board[i][j] * 32 - 32, 0, 32, 36);
			assets->tiles.setTextureRect(rect);
			assets->tiles.setPosition(j * 32, i * 32 + 28);
			assets->tiles.move(32, 32);

			window->draw(assets->tiles);
		}
	}
	for (int i = 0; i < 4; i++)
	{
		if (tetro_y[i] < 0)
		{
			continue;
		}
		sf::IntRect rect = sf::IntRect(tetro_type * 32 - 32, 0, 32, 36);
		assets->tiles.setTextureRect(rect);
		assets->tiles.setPosition(tetro_x[i] * 32, tetro_y[i] * 32 + 28);
		assets->tiles.move(32, 30);

		window->draw(assets->tiles);
	}
	
	assets->text.setFillColor(sf::Color(153, 71, 0));

	assets->text.setString(std::to_string(lines));
	assets->text.setPosition(458, 516);
	assets->text.setOrigin(assets->text.getGlobalBounds().width / 2, 0);
	window->draw(assets->text);

	assets->text.setString(std::to_string(score));
	assets->text.setPosition(458, 580);
	assets->text.setOrigin(assets->text.getGlobalBounds().width / 2, 0);
	window->draw(assets->text);
}

int Board::checkLine()
{
    int k = board_row - 1;
	int sum = 100;
	int ret = 0;

    for (int i = board_row - 1; i > 0; i--)
    {
        int count = 0;
        for (int j = 0; j < board_col; j++)
        {
            if (board[i][j])
			{
                count++;
			}
            board[k][j] = board[i][j];
        }
        if (count < board_col)
		{
            k--;
		}
		else
		{
			ret += sum;
			lines += 1;
			sum += 100;
		}
    }
	score += ret;
	return ret;
}

double Board::searchMax(int tet, std::set<unsigned long long> &set)
{
	if (!set.insert(this->hashBoard()).second)
	{
		return 0.0;
	}

	double res = 0, ret = 0;
	int tmp_x[4], tmp_y[4];

	for (int i = 0; i < 4; i++)
    {
        tmp_x[i] = tetro_x[i];
		tmp_y[i] = tetro_y[i];
    }

	if (this->dropTetro())
	{
		if (!tet)
		{
			ret = this->evaluateBoard();
			for (int i = 0; i < 4; i++)
			{
				if (tetro_y[i] < 0)
				{
					return 0.0;
				}
				board[tetro_y[i]][tetro_x[i]] = 0;
			}
			return ret;
		}
		else
		{
			Board emu(board_row, board_col, tet, board);
			for (int i = 0; i < 4; i++)
			{
				if (tetro_y[i] < 0)
				{
					return 0.0;
				}
				board[tetro_y[i]][tetro_x[i]] = 0;
			}
			std::set<unsigned long long> set2;
			res = emu.checkLine();
			res += emu.searchMax(0, set2);
			ret = std::max(res, ret);
		}
	}
	else
	{
		res = this->searchMax(tet, set);
		ret = std::max(res, ret);
		for (int i = 0; i < 4; i++)
		{
			tetro_x[i] = tmp_x[i];
			tetro_y[i] = tmp_y[i];
		}
	}

	this->moveTetro(-1);
	res = this->searchMax(tet, set);
	ret = std::max(res, ret);
	for (int i = 0; i < 4; i++)
	{
		tetro_x[i] = tmp_x[i];
		tetro_y[i] = tmp_y[i];
	}

	this->moveTetro(1);
	res = this->searchMax(tet, set);
	ret = std::max(res, ret);
	for (int i = 0; i < 4; i++)
	{
		tetro_x[i] = tmp_x[i];
		tetro_y[i] = tmp_y[i];
	}

	this->rotateTetro(-1);
	res = this->searchMax(tet, set);
	ret = std::max(res, ret);
	for (int i = 0; i < 4; i++)
	{
		tetro_x[i] = tmp_x[i];
		tetro_y[i] = tmp_y[i];
	}

	this->rotateTetro(1);
	res = this->searchMax(tet, set);
	ret = std::max(res, ret);
	for (int i = 0; i < 4; i++)
	{
		tetro_x[i] = tmp_x[i];
		tetro_y[i] = tmp_y[i];
	}

	return ret;
}

int Board::selectTetro()
{
	double min = 1e20;
	int ret = 0;

	for (int i = 1; i <= 7; i++)
	{
		std::set<unsigned long long> set;
		Board emu(board_row, board_col, i, board);
		double res = emu.searchMax(0, set);

		printf("%lf\n\n", tetro_fac[i - 1] * res);

		if (tetro_fac[i - 1] * res < min)
		{
			min = res * tetro_fac[i - 1];
			ret = i;
		}
	}
	return ret;
}

void Board::initTetro(int tet)
{
	const int tetros[7][4] =
	{
		3, 5, 4, 6,   // S
		2, 4, 5, 7,   // Z
		2, 3, 5, 7,   // L
		3, 5, 7, 6,   // J
		3, 5, 4, 7,   // T
		1, 3, 5, 7,   // I
		2, 3, 4, 5,   // O
	};

	tetro_type = tet;

	for (int i = 0; i < 7; i++)
	{
		if (i != tetro_type - 1)
		{
			tetro_fac[i] *= DECLINE;
		}
		else
		{
			tetro_fac[i] = 1.0;
		}
	}

	int n = tetro_type - 1;
	
	for (int i = 0; i < 4; i++)
	{
		tetro_x[i] = tetros[n][i] % 2;
		tetro_y[i] = tetros[n][i] / 2 - 3;
	}
}

void Board::moveTetro(int dx)
{
	bool flag = false;
	
    for (int i = 0; i < 4; i++)
    {
        tetro_x[i] += dx;

		if (tetro_x[i] < 0
			 || tetro_x[i] >= board_col
			 || (tetro_y[i] >= 0
			 && board[tetro_y[i]][tetro_x[i]]))
		{
			flag = true;
		}
    }
    if (flag)
	{
		for (int i = 0; i < 4; i++)
		{
			tetro_x[i] -= dx;
		}
	}
}

void Board::rotateTetro(int dx)
{
	if (tetro_type == 7)
	{
		return;
	}

	bool flag = true;
    int tmp_x[4], tmp_y[4];

	for (int i = 0; i < 4; i++)
    {
        tmp_x[i] = tetro_x[i];
		tmp_y[i] = tetro_y[i];
    }
    for (int i = 1; i <= 4; i++)
    {
		bool flag2 = true;

		for (int j = 0; j < 4; j++)
		{
			tetro_x[j] = tmp_x[i % 4] + dx * (tmp_y[i % 4] - tmp_y[j]);
        	tetro_y[j] = dx * (tmp_x[j] - tmp_x[i % 4]) + tmp_y[i % 4];

			if (tetro_x[j] < 0
				 || tetro_x[j] >= board_col
				 || (tetro_y[j] >= 0
				 && (tetro_y[j] >= board_row
				 || board[tetro_y[j]][tetro_x[j]])))
			{
				flag2 = false;
				break;
			}
		}
        if (flag2)
		{
			flag = false;
			break;
		}
    }
    
    if (flag)
	{
		for (int i = 0; i < 4; i++)
		{
			tetro_x[i] = tmp_x[i];
			tetro_y[i] = tmp_y[i];
		}
	}
}

bool Board::dropTetro()
{
	bool flag = false;

    for (int i = 0; i < 4; i++)
    {
        tetro_y[i] += 1;

		if (tetro_y[i] >= 1
			 && (tetro_y[i] >= board_row
			 || board[tetro_y[i]][tetro_x[i]]))
		{
			flag = true;
		}
    }
    if (flag)
    {
        for (int i = 0; i < 4; i++)
        {
			tetro_y[i]--;
			if (tetro_y[i] < 0)
			{
				continue;
			}
            board[tetro_y[i]][tetro_x[i]] = tetro_type;
        }
    }
	return flag;
}

void Board::harddropTetro()
{
	while (!this->dropTetro());
}