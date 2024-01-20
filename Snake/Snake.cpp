#include <windows.h>
#include <iostream>
#include <conio.h>
#include <chrono>
#include <random>
#include <thread>
#include <list>

namespace
{
    constexpr uint8_t WIDTH  = 15;
    constexpr uint8_t HEIGHT = 10;

    constexpr uint8_t EASY_DELAY   = 1000;
    constexpr uint8_t MEDIUM_DELAY = 600;
    constexpr uint8_t HARD_DELAY   = 200;

    uint8_t DELAY;

    std::list<uint8_t> directions;
    uint8_t direction = 0;

    uint8_t get_direction() 
    {
        uint8_t res = direction;

        if (size(directions)) {
            res = directions.front();
            directions.clear();
        }

        return direction = res;
    }

    struct point 
    {
        uint8_t x, y;
        point() = default;
		point(uint8_t x, uint8_t y) : x(x), y(y) {}
    };

    enum type
    {
        space_t,
        snake_t,
        fruit_t
    };

    std::mt19937 rnd(time(0));

    struct Field;

    struct Snake 
    {
        std::list<point> body;
        Snake() = default;
        Snake(point p) {
            add_to_head(p);
        }
        point head() const {
            return body.front();
        }
        point tail() const {
            return body.back();
        }
        uint8_t size() const {
            return std::size(body);
        }
        point get_new_head() const 
        {
            point new_head;

            switch (get_direction()) {
            case 0:
                new_head = { head().x, uint8_t((head().y + 1u) % WIDTH) };
                break;
            case 1:
                new_head = { uint8_t((head().x + HEIGHT - 1u) % HEIGHT), head().y };
                break;
            case 2:
                new_head = { head().x, uint8_t((head().y + WIDTH - 1u) % WIDTH) };
                break;
            case 3:
                new_head = { uint8_t((head().x + 1u) % HEIGHT), head().y };
                break;
            }

            return new_head;
        }
        void add_to_head(point new_head) {
            body.push_front(new_head);
        }
        void erase_tail() {
            body.pop_back();
        }
    } snake;

    struct Field
    {
        uint8_t data[HEIGHT][WIDTH] = {};
        Field() = default;
        uint8_t get(point p) const {
            return data[p.x][p.y];
        }
        void set(point p, uint8_t value) {
            data[p.x][p.y] = value;
        }
        void create()
        {
            for (uint8_t i = 0; i < HEIGHT; i++) {
                for (uint8_t j = 0; j < WIDTH; j++) {
                    set({ i, j }, space_t);
                }
            }

            snake.add_to_head({ rnd() % HEIGHT, rnd() % WIDTH });

            generate_fruit();
        }
        bool generate_fruit()
        {
            if (snake.size() == WIDTH * HEIGHT) {
                return true;
            }

            uint8_t kol = rnd() % (WIDTH * HEIGHT - snake.size()) + 1;
            point cur{ 0, 0 };

            for (;;) {
                if (get(cur) == space_t) {
                    kol--;
                }
                if (kol == 0) {
                    set(cur, fruit_t);
                    break;
                }
                cur.y++;
                if (cur.y == WIDTH) {
                    cur.y = 0;
                    cur.x++;
                }
            }

            return false;
        }
        void print()
        {
            std::string res;

            res += "Score: " + std::to_string(snake.size()) + "\n";
            res += std::string(WIDTH + 2, '-') + '\n';
            for (uint8_t i = 0; i < HEIGHT; i++) {
                res += '|';
                for (uint8_t j = 0; j < WIDTH; j++) {
                    switch (get({ i, j })) {
                    case space_t:
                        res += ' ';
                        break;
                    case snake_t:
                        res += '$';
                        break;
                    case fruit_t:
                        res += '@';
                        break;
                    }
                }
                res += "|\n";
            }
            res += std::string(WIDTH + 2, '-') + '\n';

            std::cout << res;
        }
    }field;

    void end_win() {
        std::cout << "YOU WIN!\n";
    }

    void end_lose() {
        std::cout << "\nYOUR SCORE - " << std::to_string(snake.size()) << "\n";
    }

    void set_window_size() 
    {
        HWND console = GetConsoleWindow();
        RECT r;
        GetWindowRect(console, &r);

        MoveWindow(console, r.left, r.top, (WIDTH + 2) * 10, (HEIGHT + 4) * 19, TRUE);
    }

    void input() 
    {
        for (;;) {
            switch (_getch()) {
            case 'a':
                if (direction != 0 or snake.size() == 1) {
                    directions.push_back(2);
                }
                break;
            case 'w':
                if (direction != 3 or snake.size() == 1) {
                    directions.push_back(1);
                }
                break;
            case 'd':
                if (direction != 2 or snake.size() == 1) {
                    directions.push_back(0);
                }
                break;
            case 's':
                if (direction != 1 or snake.size() == 1) {
                    directions.push_back(3);
                }
                break;
            }
        }
    }

    void output() 
    {
        bool run = true;

        for (;;) {
            point new_head = snake.get_new_head();

            switch (field.get(new_head)) {
            case space_t:
                field.set(snake.tail(), space_t);
                snake.erase_tail();
                snake.add_to_head(new_head);
                field.set(snake.head(), snake_t);
                break;
            case snake_t:
                end_lose();
                run = false;
                break;
            case fruit_t:
                snake.add_to_head(new_head);
                field.set(snake.head(), snake_t);
                if (field.generate_fruit()) {
                    end_win();
                    run = false;
                }
                break;
            }

            if (run == false) {
                break;
            }

            field.print();

            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
        }
    }

    void difficulty_coosing() 
    {
        std::cout << "Choose difficulty:\n";
        std::cout << "1 - Easy\n";
        std::cout << "2 - Medium\n";
        std::cout << "3 - Hard\n";

        for (;;) {
            switch (_getch()) {
            case '1':
                DELAY = EASY_DELAY;
                return;
            case '2':
                DELAY = MEDIUM_DELAY;
                return;
            case '3':
                DELAY = HARD_DELAY;
                return;
            }
        }
    }
}

int main()
{
    std::ios::sync_with_stdio(0), std::cin.tie(0);

    set_window_size();

    difficulty_coosing();

    field.create();

    std::thread input_thread(input);
    std::thread output_thread(output);

    input_thread.join();
    output_thread.join();
}