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

		int **_board, **board;
		int board_row;
		int board_col;

		int tetro_x[4];
		int tetro_y[4];
		int tetro_type;
		int rot;

		int score;
		int lines;

		int queue;

		int state;

		double occur[7];

		unsigned long long *_hashline, *hashline;
		bool *_change, *change;

	public:

		Board(int row, int col, int tetro, int** begin_board = NULL, int que = 0);
		~Board();

		void updateHash();

		unsigned long long hashBoard();

		int evaluateBoard();

		void drawBoard(sf::RenderWindow *window, Assets *assets);

		int checkLine();

		double searchMax(int tet, std::set<unsigned long long> &set, int max);

		int selectTetro();

		void initTetro(int tet);

		void rotateTetro(int dx);

		void moveTetro(int dx);

		bool dropTetro();

		void updateBest(int *table, int pos);

		int getState() { return state; }

		void setState(int stat) { state = stat; }
};

void drawMisc(Assets *assets, sf::RenderWindow *window, int level, bool hard, int *table)
{
	assets->text.setString("Lvl " + std::to_string(level));	
	assets->text.setPosition(462, 616);
	assets->text.setOrigin(assets->text.getGlobalBounds().width / 2, 0);
	window->draw(assets->text);

	assets->text.setString("High:");	
	assets->text.setPosition(462, 646);
	assets->text.setOrigin(assets->text.getGlobalBounds().width / 2, 0);
	window->draw(assets->text);

	assets->text.setString(std::to_string(table[level + hard * (MAXLEVEL + 1)]));
	assets->text.setPosition(458, 674);
	assets->text.setOrigin(assets->text.getGlobalBounds().width / 2, 0);
	window->draw(assets->text);
}

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

	sf::Music music;
	music.openFromFile("Naughty.ogg");
	music.setLoop(true);

	sf::SoundBuffer statebuf[4];
	statebuf[0].loadFromFile("Begin.wav");
	statebuf[2].loadFromFile("Pause.wav");
	statebuf[3].loadFromFile("Die.wav");
	sf::Sound state[4];
	for (int i = 0; i < 4; i++)
	{
		state[i].setBuffer(statebuf[i]);
	}

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
	bool harddrop = false;
	bool play = true;
	double fac[10] = {1e20, 1.0};
	int level = 1, tmp_level = 1;

	for (int i = 2; i < 10; i++)
	{
		fac[i] = fac[i - 1] / 1.5;
	}

	while (window.isOpen())
	{
		if (board->getState() == 1)
		{
			if (music.getStatus() != sf::SoundSource::Status::Playing)
			{
				state[2].setPitch(level ? 0.53 / fac[level > 5 ? 5 : level] : 0.53);
				state[3].setPitch(level ? 0.53 / fac[level > 5 ? 5 : level] : 0.53);
				music.setPitch(level ? 0.53 / fac[level > 5 ? 5 : level] : 0.53);
				music.play();
			}
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

						delay = level ? speed * fac[level] * 0.125 : 0;
						harddrop = true;
						break;
					
					case sf::Keyboard::P:

						board->setState(2);
						break;
					}
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
				{
					delay = level ? speed * fac[level] * 0.125 : 0;
				}
			}
			if (timer > delay)
			{
				if (harddrop)
				{
					while (!board->dropTetro());
				}
				if (harddrop || board->dropTetro())
				{
					if (board->getState() == 1)
					{
						board->initTetro(board->selectTetro());
						play = true;
					}
					harddrop = false;
				}
				timer = 0;
			}
			board->checkLine();
			board->updateBest(table, level + hard * (MAXLEVEL + 1));

			board->drawBoard(&window, &assets);

			drawMisc(&assets, &window, level, hard, table);
		}
		else
		{
			if (board->getState() == 2)
			{
				music.pause();
			}
			else
			{
				music.stop();
			}
			if (play)
			{
				state[board->getState()].play();
				play = false;
			}
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
							state[2].stop();
							board->setState(1);
							play = true;
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
					state[board->getState()].stop();
					board = new Board(20, 10, rand() % 4 + 3, NULL, que);
					board->setState(1);
					play = true;
				}
			}

			board->drawBoard(&window, &assets);

			assets.menu.setPosition(64, 288);
			window.draw(assets.menu);

			std::string instruction = "Space: choose\n\nEnter: start";
			if (board->getState() == 0)
			{
				assets.text.setString(instruction);
			}
			else if (board->getState() == 2)
			{
				assets.text.setString("P: continue\n\n" + instruction);
			}
			else
			{
				assets.text.setString("You lose!\n\n" + instruction);
			}
			assets.text.setPosition(192, 312);
			assets.text.setOrigin(assets.text.getGlobalBounds().width / 2, 0);
			window.draw(assets.text);

			assets.text.setString("*");	
			assets.text.setPosition(140 + hard * 100, 440);
			assets.text.setOrigin(assets.text.getGlobalBounds().width / 2, 0);
			window.draw(assets.text);

			drawMisc(&assets, &window, level, hard, table);
		}
		window.display(); 
	}

	return 0;
}

Board::Board(int row, int col, int tetro, int** begin_board, int que)
{
	_board = new int*[row + 5];

	for (int i = 0; i < row + 5; i++)
	{
		_board[i] = new int[col]();
	}
	_hashline = new unsigned long long[row + 5]();
	_change = new bool[row + 5]();

	board = _board + 4;
	hashline = _hashline + 4;
	change = _change + 4;

	board_row = row;
	board_col = col;

	score = lines = 0;

	queue = que;
	initTetro(tetro);

	for (int i = 0; i < 7; i++)
	{
		occur[i] = 1.0;
	}

	if (begin_board)
	{
		for (int i = 0; i < row; i++)
		{
			memcpy(board[i], begin_board[i], sizeof(int) * col);
			change[i] = true;
		}
		updateHash();
	}
}

Board::~Board()
{
	for (int i = 0; i < board_row; i++)
	{
		delete[] _board[i];
	}
	delete[] _board;
	delete[] _hashline;
	delete[] _change;
}

void Board::updateHash()
{
	unsigned long long hash = 0;
	for (int i = 0; i < 4; i++)
	{
		if (tetro_y[i] < 0 || board[tetro_y[i]][tetro_x[i]])
		{
			continue;
		}
		board[tetro_y[i]][tetro_x[i]] = 8;
	}
	for (int i = 0; i < board_row; i++)
	{
		if (change[i])
		{
			hashline[i] = 0;
			for (int j = 0; j < board_col; j++)
			{
				hashline[i] = hashline[i] * 11 + board[i][j];
			}
			change[i] = false;
		}
	}
	for (int i = 0; i < 4; i++)
	{
		if (tetro_y[i] < 0 || board[tetro_y[i]][tetro_x[i]] != 8)
		{
			continue;
		}
		board[tetro_y[i]][tetro_x[i]] = 0;
	}
}

unsigned long long Board::hashBoard()
{
	unsigned long long hash = 0;

	for (int i = 0; i < board_row; i++)
	{
		hash = hash * 25937424601ull + hashline[i];
	}
	
	return hash;
}

int Board::evaluateBoard()
{
	int ret = checkLine() * 100, min = board_row;

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

	for (int i = 0; i < 4; i++)
	{
		board[tetro_y[i]][tetro_x[i]] = tetro_type;
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
		board[tetro_y[i]][tetro_x[i]] = 0;
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
			ret += sum;
			sum += 100;
		}
		change[i] = 1;
	}
	updateHash();
	return ret;
}

#define UPDATE {\
	res = searchMax(tet, set, dep + 1);\
	ret = std::max(res, ret);\
	for (int i = 0; i < 4; i++)\
	{\
		change[tetro_y[i]] = true;\
		tetro_x[i] = tmp_x[i];\
		tetro_y[i] = tmp_y[i];\
		change[tetro_y[i]] = true;\
	}\
	updateHash();\
}

double Board::searchMax(int tet, std::set<unsigned long long> &set, int dep)
{
	
	if (!set.insert(hashBoard()).second)
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

	if (dropTetro())
	{
		Board emu(board_row, board_col, tet ? tet : 1, board);
		bool flag = false;
	
		for (int i = 0; i < 4; i++)
		{
			board[tetro_y[i]][tetro_x[i]] = 0;
			change[tetro_y[i]] = true;
			if (tetro_y[i] <= 0)
			{
				flag = true;
			}
		}
		updateHash();

		if (!tet)
		{
			return flag ? 0.0 : emu.evaluateBoard();
		}
		std::set<unsigned long long> set2;
		res = emu.checkLine() * 100;
		res += emu.searchMax(0, set2, dep + 1);
		ret = std::max(res, ret);
	}
	else UPDATE

	moveTetro(-1);
	UPDATE
	moveTetro(1);
	UPDATE
	rotateTetro(-1);
	UPDATE
	rotateTetro(1);
	UPDATE

	return ret;
}

#undef UPDATE

int Board::selectTetro()
{
	double min = 1e20, min2 = 1e20;
	int ret = 0, ret2 = 0;

	for (int i = 1; i <= 7; i++)
	{
		std::set<unsigned long long> set;
		Board emu(board_row, board_col, queue ? queue : i, board);
		double res = emu.searchMax(queue ? i : 0, set, 0) - occur[i - 1];

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
		5, 0, 1, 6,   // S
		5, 1, 2, 4,   // Z
		5, 2, 4, 6,   // L
		5, 0, 4, 6,   // J
		5, 1, 4, 6,   // T
		0, 1, 2, 3,   // I
		1, 2, 5, 6,   // O
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

	rot = 0;

	int n = tetro_type - 1;
	
	for (int i = 0; i < 4; i++)
	{
		tetro_x[i] = tetros[n][i] % 4 + 3;
		tetro_y[i] = tetros[n][i] / 4 - (n != 5);
	}
}

void Board::moveTetro(int dx)
{
	bool flag = false;
	
	for (int i = 0; i < 4; i++)
	{
		tetro_x[i] += dx;
		change[tetro_y[i]] = true;

		if (tetro_x[i] < 0
			 || tetro_x[i] >= board_col
			 || board[tetro_y[i]][tetro_x[i]])
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
	updateHash();
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
		change[tetro_y[i]] = true;
	}
	if (dx == -1)
	{
		rot = (rot + 3) % 4;
	}

	if (tetro_type == 6)
	{
		int ind = dx == 1 ? 1 + (rot & 1) : 2 - (rot & 1);
		int fx = dx * ((rot ^ (rot >> 1)) & 1 ? -1 : 1);
		int fy = dx * (rot & 2 ? -1 : 1);
		int tx = tmp_x[ind], ty = tmp_y[ind];
		int cx[5] = {0, -2 * fx, fx, -2 * fx, fx};
		int cy[5] = {0, 0, 0, -fy, 2 * fy};

		if (rot & 1)
		{
			std::swap(cx[1], cx[2]);
			std::swap(cx[3], cy[3]);
			std::swap(cx[4], cy[4]);
		}

		for (int i = 0; i < 5; i++)
		{
			bool flag2 = true;

			for (int j = 0; j < 4; j++)
			{
				tetro_x[j] = tx - dx * (ty - tmp_y[j]) + cx[i];
				tetro_y[j] = ty - dx * (tmp_x[j] - tx) - cy[i];

				if (tetro_x[j] < 0
					|| tetro_x[j] >= board_col
					|| tetro_y[j] >= board_row
					|| board[tetro_y[j]][tetro_x[j]])
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
	}
	else
	{
		int tx = tmp_x[0], ty = tmp_y[0];
		int fx = dx * ((rot ^ (rot >> 1)) & 1 ? -1 : 1);
		int fy = dx * (rot & 1 ? -1 : 1);
		int cx[5] = {0, -fx, -fx, 0, -fx};
		int cy[5] = {0, 0, -fy, 2 * fy, 2 * fy};

		for (int i = 0; i < 5; i++)
		{
			bool flag2 = true;
			
			for (int j = 0; j < 4; j++)
			{
				tetro_x[j] = tx + dx * (ty - tmp_y[j]) + cx[i];
				tetro_y[j] = dx * (tmp_x[j] - tx) + ty + cy[i];

				if (tetro_x[j] < 0
					|| tetro_x[j] >= board_col
					|| tetro_y[j] >= board_row
					|| board[tetro_y[j]][tetro_x[j]])
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
	}
	if (flag)
	{
		for (int i = 0; i < 4; i++)
		{
			tetro_x[i] = tmp_x[i];
			tetro_y[i] = tmp_y[i];
		}
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			change[tetro_y[i]] = true;
		}
	}
	updateHash();

	if ((dx == 1) ^ flag)
	{
		rot = (rot + 1) % 4;
	}
}

bool Board::dropTetro()
{
	bool flag = false;

	for (int i = 0; i < 4; i++)
	{
		tetro_y[i] += 1;
		change[tetro_y[i]] = true;
		change[tetro_y[i] - 1] = true;
		if (tetro_y[i] == board_row
			 || board[tetro_y[i]][tetro_x[i]])
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
			}
			if (tetro_y[i] >= 0)
			{
				board[tetro_y[i]][tetro_x[i]] = tetro_type;
			}
		}
	}
	updateHash();

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