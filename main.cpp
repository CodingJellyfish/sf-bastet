#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <cstdlib>
#include <cstring>
#include <ctime>

#include <algorithm>
#include <fstream>
#include <set>
#include <string>
#include <utility>

const int MAXLEVEL = 9;

struct Assets
{
	sf::Sprite board;
	sf::Sprite tiles;
	sf::Sprite menu;
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

		int queue;

		int state;

		double occur[7];

	public:

		Board(int row, int col, int tetro, int** begin_board = NULL, int que = 0);
		~Board();

		unsigned hashBoard();

		int evaluateBoard();

		void drawBoard(sf::RenderWindow *window, Assets *assets);

		int checkLine();

		double searchMax(int tet, std::set<unsigned> &set);

		int selectTetro();

		void initTetro(int tet);

		void rotateTetro(int dx);

		void moveTetro(int dx);

		bool dropTetro();

		void updateBest(int *table, int pos);

		int getState() { return state; }

		void setState(int stat) { state = stat; }
};

int main()
{
	srand(time(0));

	sf::RenderWindow window(sf::VideoMode(576, 768), "Bastet!");

	double speed = 1.0;

	Board *board = new Board(20, 10, 0);
	board->setState(0);

	sf::Clock clock;
	Assets assets;

	sf::Texture tex_board;
	tex_board.loadFromFile("background.png");
	assets.board = sf::Sprite(tex_board);

	sf::Texture tex_tiles;
	tex_tiles.loadFromFile("tiles.png");
	assets.tiles = sf::Sprite(tex_tiles);

	sf::Texture tex_menu;
	tex_menu.loadFromFile("menu.png");
	assets.menu = sf::Sprite(tex_menu);

	sf::Font font;
	font.loadFromFile("dogicapixelbold.ttf");
	assets.text = sf::Text("", font, 18);

	sf::Image icon;
	icon.loadFromFile("icon.png");
	window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

	std::ifstream istream("best.txt");
	int table[(MAXLEVEL + 1) * 2];
	int checker;
	istream >> checker;

	if (checker != 114514)
	{
		std::ofstream stream("best.txt");
		stream << 114514 << " ";
		for (int i = 0; i < (MAXLEVEL + 1) * 2; i++)
		{
			stream << 0 << " ";
		}
		memset(table, 0, sizeof(table));
	}
	else
	{
		for (int i = 0; i < 20; i++)
		{
			istream >> table[i];
		}
	}

	double timer = 0;
	double delay = 0;
	bool hard = false;
	double fac[10] = {1.0};
	int level = 0, tmp_level = 0;

	for (int i = 1; i < 10; i++)
	{
		fac[i] = fac[i - 1] / 1.5;
	}

	while (window.isOpen())
	{
		if (board->getState() == 1)
		{
			delay = speed * fac[level];

			float time = clock.getElapsedTime().asSeconds();
			timer += time;
			tmp_level = level;
			clock.restart();

			sf::Event e;

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

						board->rotateTetro(-1);
						break;

					case sf::Keyboard::Space:

						board->rotateTetro(1);
						break;

					case sf::Keyboard::Left:

						board->moveTetro(-1);
						break;

					case sf::Keyboard::Right:

						board->moveTetro(1);
						break;

					case sf::Keyboard::Enter:

						while (!board->dropTetro());
						if (board->getState() == 1)
						{
							board->initTetro(board->selectTetro());
						}
						break;
					
					case sf::Keyboard::P:

						board->setState(2);
						break;
					}
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
				{
					delay = speed * fac[level] * 0.1;
				}
			}
			if (timer > delay)
			{
				if (board->dropTetro())
				{
					if (board->getState() == 1)
					{
						board->initTetro(board->selectTetro());
					}
				}
				timer = 0;
			}
			board->checkLine();
			board->updateBest(table, level + hard * (MAXLEVEL + 1));

			board->drawBoard(&window, &assets);

			assets.text.setString("Lvl " + std::to_string(level));	
			assets.text.setPosition(462, 616);
			assets.text.setOrigin(assets.text.getGlobalBounds().width / 2, 0);
			window.draw(assets.text);

			assets.text.setString("High:");	
			assets.text.setPosition(462, 646);
			assets.text.setOrigin(assets.text.getGlobalBounds().width / 2, 0);
			window.draw(assets.text);

			assets.text.setString(std::to_string(table[level + hard * (MAXLEVEL + 1)]));
			assets.text.setPosition(462, 674);
			assets.text.setOrigin(assets.text.getGlobalBounds().width / 2, 0);
			window.draw(assets.text);
		}
		else
		{
			sf::Event e;
			bool start = false;

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
					case sf::Keyboard::Space:

						hard = !hard;
						break;

					case sf::Keyboard::Enter:

						start = true;
						break;
					
					case sf::Keyboard::P:

						if (board->getState() == 2)
						{
							level = tmp_level;
							board->setState(1);
						}
						break;
					
					case sf::Keyboard::Up:

						level = level < 9 ? level + 1 : 9;
						break;
					
					case sf::Keyboard::Down:

						level = level ? level - 1 : 0;
						break;
					}
				}

				if (start)
				{
					delete board;
					int que = hard ? 0 : rand() % 7 + 1;
					board = new Board(20, 10, rand() % 4 + 3, NULL, que);
					board->setState(1);
				}
			}

			board->drawBoard(&window, &assets);

			assets.menu.setPosition(64, 288);
			window.draw(assets.menu);

			if (board->getState() == 0)
			{
				assets.text.setString("Space: choose\n\nEnter: start");
			}
			else if (board->getState() == 2)
			{
				assets.text.setString("P: continue\n\nSpace: choose\n\nEnter: restart");
			}
			else
			{
				assets.text.setString("You lose!\n\nSpace: choose\n\nEnter: restart");
			}
			assets.text.setPosition(192, 312);
			assets.text.setOrigin(assets.text.getGlobalBounds().width / 2, 0);
			window.draw(assets.text);

			assets.text.setString("*");	
			assets.text.setPosition(140 + hard * 100, 440);
			assets.text.setOrigin(assets.text.getGlobalBounds().width / 2, 0);
			window.draw(assets.text);

			assets.text.setString("Lvl " + std::to_string(level));	
			assets.text.setPosition(462, 616);
			assets.text.setOrigin(assets.text.getGlobalBounds().width / 2, 0);
			window.draw(assets.text);

			assets.text.setString("High:");	
			assets.text.setPosition(462, 646);
			assets.text.setOrigin(assets.text.getGlobalBounds().width / 2, 0);
			window.draw(assets.text);

			assets.text.setString(std::to_string(table[level + hard * (MAXLEVEL + 1)]));
			assets.text.setPosition(462, 674);
			assets.text.setOrigin(assets.text.getGlobalBounds().width / 2, 0);
			window.draw(assets.text);
		}
		window.display(); 
	}

	return 0;
}

Board::Board(int row, int col, int tetro, int** begin_board, int que)
{
	board = new int*[row];

	for (int i = 0; i < row; i++)
	{
		board[i] = new int[col]();
	}

	board_row = row;
	board_col = col;

	score = lines = 0;

	queue = que;
	this->initTetro(tetro);

	for (int i = 0; i < 7; i++)
	{
		occur[i] = 1.0;
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

unsigned Board::hashBoard()
{
	unsigned hash = 0;
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
	int ret = 0, min = board_row;

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
			ret += 10000;
		}
	}
	for (int i = 0; i < board_col; i++)
	{
		int cnt = 0;
		while (cnt < board_row && !board[cnt][i])
		{
			cnt++;
		}
		ret += cnt * 10;
		min = std::min(min, cnt);
	}
	return ret + min;
}

void Board::drawBoard(sf::RenderWindow *window, Assets *assets)
{
	window->draw(assets->board);

	assets->text.setFillColor(sf::Color(153, 71, 0));

	assets->text.setString(std::to_string(lines));
	assets->text.setPosition(458, 516);
	assets->text.setOrigin(assets->text.getGlobalBounds().width / 2, 0);
	window->draw(assets->text);

	assets->text.setString(std::to_string(score));
	assets->text.setPosition(458, 580);
	assets->text.setOrigin(assets->text.getGlobalBounds().width / 2, 0);
	window->draw(assets->text);

	if (queue)
	{
		sf::IntRect rect = sf::IntRect(queue * 32 - 32, 0, 32, 36);
		assets->tiles.setTextureRect(rect);
		assets->tiles.setPosition(440, 408);
		assets->tiles.setScale(1.5f, 1.5f);
		window->draw(assets->tiles);
		assets->tiles.setScale(1.0f, 1.0f);
	}
	else
	{
		assets->text.setString("?");
		assets->text.setPosition(460, 420);
		assets->text.setOrigin(assets->text.getGlobalBounds().width / 2, 0);
		window->draw(assets->text);
	}

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
	if (tetro_type == 0)
	{
		return;
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
}

int Board::checkLine()
{
	int k = board_row - 1;
	int sum = 100;
	int ret = 0;

	for (int i = board_row - 1; i >= 0; i--)
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
			score += sum;
			lines += 1;
			sum += 100;
			ret += 1;
		}
	}
	return ret;
}

double Board::searchMax(int tet, std::set<unsigned> &set)
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
			std::set<unsigned> set2;
			res = emu.checkLine() * 1000 - 600;
			if (res < 0)
			{
				res = 0;
			}
			res = emu.searchMax(0, set2);
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
	double min = 1e20, min2 = 1e20;
	int ret = 0, ret2 = 0;

	for (int i = 1; i <= 7; i++)
	{
		std::set<unsigned> set;
		Board emu(board_row, board_col, queue ? queue : i, board);
		double res = emu.searchMax(queue ? i : 0, set) - occur[i - 1];
		if (res < min)
		{
			min2 = min;
			ret2 = ret;
			min = res;
			ret = i;
		}
		else if (res < min2)
		{
			min2 = res;
			ret2 = i;
		}
	}
	return rand() % 5 ? ret : ret2;
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

	for (int i = 0; i < 7; i++)
	{
		if (i != tet - 1)
		{
			occur[tet - 1] /= 1.2;
			occur[i] *= 1.2;
		}
	}

	if (queue)
	{
		tetro_type = queue;
		queue = tet;
	}
	else
	{
		tetro_type = tet;
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
			if (tetro_y[i] <= 0)
			{
				state = 3;
				if (tetro_y[i] < 0)
				{
					continue;
				}
			}
			board[tetro_y[i]][tetro_x[i]] = tetro_type;
		}
	}
	return flag;
}

void Board::updateBest(int *table, int pos)
{
	std::ofstream stream("best.txt");
	if (score > table[pos])
	{
		table[pos] = score;
	}
	stream << 114514 << " ";
	for (int i = 0; i < (MAXLEVEL + 1) * 2; i++)
	{
		stream << table[i] << " ";
	}
}