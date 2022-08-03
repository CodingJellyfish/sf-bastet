#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <cstdlib>
#include <cstring>
#include <ctime>

#include <algorithm>
#include <fstream>
#include <unordered_set>
#include <string>
#include <utility>

const int MAXLEVEL = 9;

unsigned long long fac11[1000];

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

		unsigned long long hash;

	public:

		Board(int row, int col, int tetro, int** begin_board = NULL, int que = 0);
		~Board();

		void changeBoard(int x, int y, int val);

		unsigned long long hashBoard();

		int evaluateBoard();

		void drawBoard(sf::RenderWindow *window, Assets *assets);

		int checkLine();

		int searchMax(int tet, std::unordered_set<unsigned long long> &set);

		int selectTetro();

		void initTetro(int tet);

		void rotateTetro(int dx);

		void moveTetro(int dx);

		bool dropTetro(int *state = NULL);

		void updateBest(int *table, int pos);
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

	fac11[0] = 1;
	for (int i = 1; i < 1000; i++)
	{
		fac11[i] = fac11[i - 1] * 11;
	}

	Board *board = new Board(20, 10, 0);

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

	int stat = 0;
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
		if (stat == 1)
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

						delay = level ? speed * fac[level] * 0.125 : 0.125;
						harddrop = true;
						break;
					
					case sf::Keyboard::P:

						stat = 2;
						break;
					}
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
				{
					delay = 0;
				}
			}
			if (timer > delay)
			{
				if (harddrop)
				{
					while (!board->dropTetro(&stat));
				}
				if (harddrop || board->dropTetro(&stat))
				{
					if (stat == 1)
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
			if (stat == 2)
			{
				music.pause();
			}
			else
			{
				music.stop();
			}
			if (play)
			{
				state[stat].play();
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

						if (stat == 2)
						{
							level = tmp_level;
							state[2].stop();
							stat = 1;
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
					state[stat].stop();
					board = new Board(20, 10, rand() % 4 + 3, NULL, que);
					stat = 1;
					play = true;
				}
			}

			board->drawBoard(&window, &assets);

			assets.menu.setPosition(64, 288);
			window.draw(assets.menu);

			std::string instruction = "Space: choose\n\nEnter: start";
			if (stat == 0)
			{
				assets.text.setString(instruction);
			}
			else if (stat == 2)
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
	_board = new int*[row + 10];

	for (int i = 0; i < row + 10; i++)
	{
		_board[i] = new int[col]();
	}

	board = _board + 5;

	board_row = row;
	board_col = col;

	score = lines = 0;

	for (int i = 0; i < 7; i++)
	{
		occur[i] = 1.0;
	}

	queue = que;
	initTetro(tetro);
	
	hash = 0;

	if (begin_board)
	{
		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < col; j++)
			{
				changeBoard(j, i, begin_board[i][j]);
			}
		}
	}
}

Board::~Board()
{
	for (int i = 0; i < board_row + 10; i++)
	{
		delete[] _board[i];
	}
	delete[] _board;
}

void Board::changeBoard(int x, int y, int val)
{
	hash += fac11[y * board_col + x] * ((!!val) - (!!board[y][x]));
	board[y][x] = val;
}

unsigned long long Board::hashBoard()
{
	unsigned long long ret = hash;

	for (int i = 0; i < 4; i++)
	{
		int ind = tetro_y[i] * board_col + tetro_x[i];
		if (ind >= 0 && ind < board_row * board_col)
		{
			ret += fac11[ind] * 2;
		}
	}
	return ret;
}

int Board::evaluateBoard()
{
	int ret = checkLine() * 1000, min = board_row;

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
		sf::IntRect rect = sf::IntRect(queue * 32, 0, 32, 36);
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
		board[tetro_y[i]][tetro_x[i]] = 8;
	}

	for (int i = 0; i < board_row; i++)
	{
		for (int j = 0; j < board_col; j++)
		{
			sf::IntRect rect = sf::IntRect(board[i][j] * 32, 0, 32, 36);
			assets->tiles.setTextureRect(rect);
			assets->tiles.setPosition(j * 32, i * 32 + 32);
			assets->tiles.move(32, 28);

			window->draw(assets->tiles);
		}
	}

	bool vis[4] = {false, false, false, false};
	for (int i = 0; i < 4; i++)
	{
		int min = board_row, mini = 0;
		for (int j = 0; j < 4; j++)
		{
			if (!vis[j] && tetro_y[j] < min)
			{
				min = tetro_y[j];
				mini = j;
			}
		}
		vis[mini] = true;
		board[tetro_y[mini]][tetro_x[mini]] = 0;
		if (min < 0)
		{
			continue;
		}
		sf::IntRect rect = sf::IntRect(tetro_type * 32, 0, 32, 36);
		assets->tiles.setTextureRect(rect);
		assets->tiles.setPosition(tetro_x[mini] * 32, tetro_y[mini] * 32 + 32);
		assets->tiles.move(32, 24);

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
			changeBoard(j, k, board[i][j]);
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
	}
	return ret;
}

#define UPDATE {\
	res = searchMax(tet, set);\
	ret = std::max(res, ret);\
	for (int i = 0; i < 4; i++)\
	{\
		tetro_x[i] = tmp_x[i];\
		tetro_y[i] = tmp_y[i];\
	}\
}

int Board::searchMax(int tet, std::unordered_set<unsigned long long> &set)
{
	if (!set.insert(hashBoard()).second)
	{
		return 0.0;
	}

	int res = 0, ret = 0;
	int tmp_x[4], tmp_y[4];

	for (int i = 0; i < 4; i++)
	{
		tmp_x[i] = tetro_x[i];
		tmp_y[i] = tetro_y[i];
	}

	int state = 0;
	if (dropTetro(&state))
	{
		if (tet == 0)
		{
			Board emu(board_row, board_col, 1, board);
			ret = std::max(ret, state == 3 ? 0 : emu.evaluateBoard());
		}
		else if (state != 3)
		{
			int res2[7];

			for (int i = tet == 8 ? 1 : tet; i <= (tet == 8 ? 7 : tet); i++)
			{
				Board emu(board_row, board_col, i, board);
				std::unordered_set<unsigned long long> set2;
				res2[i - 1] = emu.checkLine() * 1010;
				res2[i - 1] += emu.searchMax(0, set2);
			}
			if (tet == 8)
			{
				std::sort(res2, res2 + 7);
				ret = std::max(ret, (int)(0.8 * res2[0] + 0.2 * res2[1]));
			}
			else
			{
				ret = std::max(ret, res2[tet - 1]);
			}
		}
		for (int i = 0; i < 4; i++)
		{
			changeBoard(tetro_x[i], tetro_y[i], 0);
		}
	}
	else UPDATE

	moveTetro(-1);
	UPDATE
	rotateTetro(-1);
	UPDATE
	moveTetro(1);
	UPDATE
	rotateTetro(1);
	UPDATE

	return ret;
}

#undef UPDATE

int Board::selectTetro()
{
	std::pair<double, int> res[7];

	for (int i = 1; i <= 7; i++)
	{
		std::unordered_set<unsigned long long> set;
		Board emu(board_row, board_col, queue ? queue : i, board);
		res[i - 1].first = emu.searchMax(queue ? i : 8, set) - occur[i - 1];
		res[i - 1].second = i;
	}
	std::sort(res, res + 7);

	return rand() % 5 ? res[0].second : res[1].second;
}

void Board::initTetro(int tet)
{
	static const int tetros[7][4] =
	{
		5, 0, 1, 6,   // S
		5, 1, 2, 4,   // Z
		5, 2, 4, 6,   // L
		5, 0, 4, 6,   // J
		5, 1, 4, 6,   // T
		0, 1, 2, 3,   // I
		1, 2, 5, 6,   // O
	};

	if (queue)
	{
		tetro_type = queue;
		queue = tet;
	}
	else
	{
		tetro_type = tet;
	}

	for (int i = 0; i < 7; i++)
	{
		if (i != tet - 1)
		{
			occur[tet - 1] /= 1.3;
			occur[i] *= 1.3;
		}
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
	if (dx == -1)
	{
		rot = (rot + 3) % 4;
	}

	int ind = tetro_type == 6 ? (dx == 1 ? 1 + (rot & 1) : 2 - (rot & 1)) : 0;
	int fx = dx * ((rot ^ (rot >> 1)) & 1 ? -1 : 1);
	int fy = dx * (rot & (1 << (tetro_type == 6)) ? -1 : 1);
	int cx[2][5] = {0, -fx, -fx, 0, -fx, 0, -2 * fx, fx, -2 * fx, fx};
	int cy[2][5] = {0, 0, fy, -2 * fy, -2 * fy, 0, 0, 0, -fy, 2 * fy};
	int t6 = tetro_type == 6 ? -1 : 1;

	if (rot & 1)
	{
		std::swap(cx[1][1], cx[1][2]);
		std::swap(cx[1][3], cy[1][3]);
		std::swap(cx[1][4], cy[1][4]);
	}

	for (int i = 0; i < 5; i++)
	{
		bool flag2 = true;

		for (int j = 0; j < 4; j++)
		{
			tetro_x[j] = tmp_x[ind] + t6 * dx * (tmp_y[ind] - tmp_y[j])
			           + cx[tetro_type == 6][i];
			tetro_y[j] = tmp_y[ind] + t6 * dx * (tmp_x[j] - tmp_x[ind])
			           - cy[tetro_type == 6][i];

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

	if (flag)
	{
		for (int i = 0; i < 4; i++)
		{
			tetro_x[i] = tmp_x[i];
			tetro_y[i] = tmp_y[i];
		}
	}
	if ((dx == 1) ^ flag)
	{
		rot = (rot + 1) % 4;
	}
}

bool Board::dropTetro(int *state)
{
	bool flag = false;

	for (int i = 0; i < 4; i++)
	{
		tetro_y[i] += 1;
		if (tetro_y[i] == board_row || board[tetro_y[i]][tetro_x[i]])
		{
			flag = true;
		}
	}
	if (flag)
	{
		for (int i = 0; i < 4; i++)
		{
			tetro_y[i]--;

			if (tetro_y[i] <= 0 && state)
			{
				*state = 3;
			}
			if (tetro_y[i] >= 0)
			{
				changeBoard(tetro_x[i], tetro_y[i], tetro_type);
			}
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