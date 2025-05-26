import pygame
import sys
import random
from paddle import Paddle
from ball import Ball
from button import Button

pygame.init()

WIDTH, HEIGHT = 1280, 720

SCREEN = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Ping-pong genius")

BG = pygame.image.load("Background.png")

FPS = 60

WHITE = (255, 255, 255)
BLACK = (0, 0, 0)

PADDLE_WIDTH, PADDLE_HEIGHT = 20, 100
BALL_RADIUS = 7
WINNING_SCORE = 1

LEVEL = 1
MAX_GAME = 1
MAX_WIN = 1
MODE = 2


# 设置字体
def get_font(size):
    return pygame.font.Font("font.ttf", size)


# 绘制面板
def draw(win, paddles, ball, left_score, right_score, left_wins, right_wins):
    # 根据不同的游戏模式赋予玩家姓名
    if MODE == 1:
        left_name, right_name = "player", "computer"
    elif MODE == 2:
        left_name, right_name = "computer", "player",
    elif MODE == 3:
        left_name, right_name = "player1", "player2",

    # 背景填充
    win.fill(BLACK)

    # 绘制姓名和对应得分情况
    left_text = get_font(20).render(f"{left_name}:{left_wins}wins {left_score}balls", True, "Blue")
    right_text = get_font(20).render(f"{right_name}:{right_wins}wins {right_score}balls", True, "Blue")

    win.blit(left_text, (WIDTH // 4 - left_text.get_width() // 2, 20))
    win.blit(right_text, (WIDTH * (3 / 4) - right_text.get_width() // 2, 20))

    # 绘制球拍
    for paddle in paddles:
        paddle.draw(win)

    # 绘制球网
    for i in range(10, HEIGHT, HEIGHT // 20):
        if (i - 10) % (HEIGHT // 10) == 0:
            continue
        pygame.draw.rect(win, WHITE, (WIDTH // 2 - 5, i, 10, HEIGHT // 20))

    # 绘制乒乓球
    ball.draw(win)
    pygame.display.update()


# 处理碰撞
def collision(ball, left_paddle, right_paddle):
    # 乒乓球撞击上边界
    if ball.y + ball.radius >= HEIGHT:
        ball.y_vel *= -1
    # 乒乓球撞击下边界
    elif ball.y - ball.radius <= 0:
        ball.y_vel *= -1

    # 乒乓球与左球拍相撞
    if ball.x_vel < 0:
        if left_paddle.y <= ball.y <= left_paddle.y + left_paddle.height:
            if ball.x - ball.radius <= left_paddle.x + left_paddle.width:
                ball.x_vel *= -1

                # 根据击球位置确定速度损失
                middle_y = left_paddle.y + left_paddle.height / 2
                difference_in_y = middle_y - ball.y
                reduction_factor = (left_paddle.height / 2) / ball.MAX_VEL
                y_vel = difference_in_y / reduction_factor
                ball.y_vel = -1 * y_vel

    # 乒乓球与右球拍相撞
    else:
        if right_paddle.y <= ball.y <= right_paddle.y + right_paddle.height:
            if ball.x + ball.radius >= right_paddle.x:
                ball.x_vel *= -1

                middle_y = right_paddle.y + right_paddle.height / 2
                difference_in_y = middle_y - ball.y
                reduction_factor = (right_paddle.height / 2) / ball.MAX_VEL
                y_vel = difference_in_y / reduction_factor
                ball.y_vel = -1 * y_vel


# 处理球拍移动
def movement(keys, left_paddle, right_paddle, ball, mode, level):
    # 若为左人右机模式
    if mode == 1:
        if keys[pygame.K_w] and left_paddle.y - left_paddle.VEL >= 0:
            left_paddle.move(up=True)
        if keys[pygame.K_s] and left_paddle.y + left_paddle.VEL + left_paddle.height <= HEIGHT:
            left_paddle.move(up=False)
        computer_movement(right_paddle, ball, mode, level)
    # 若为左机右人模式
    elif mode == 2:
        if keys[pygame.K_UP] and right_paddle.y - right_paddle.VEL >= 0:
            right_paddle.move(up=True)
        if keys[pygame.K_DOWN] and right_paddle.y + right_paddle.VEL + right_paddle.height <= HEIGHT:
            right_paddle.move(up=False)
        computer_movement(left_paddle, ball, mode, level)
    # 若为玩家对战模式
    elif mode == 3:
        if keys[pygame.K_w] and left_paddle.y - left_paddle.VEL >= 0:
            left_paddle.move(up=True)
        if keys[pygame.K_s] and left_paddle.y + left_paddle.VEL + left_paddle.height <= HEIGHT:
            left_paddle.move(up=False)

        if keys[pygame.K_UP] and right_paddle.y - right_paddle.VEL >= 0:
            right_paddle.move(up=True)
        if keys[pygame.K_DOWN] and right_paddle.y + right_paddle.VEL + right_paddle.height <= HEIGHT:
            right_paddle.move(up=False)


# 处理电脑移动球拍
def computer_movement(paddle, ball, mode, level):
    # 若为左人右机模式
    if mode == 1:
        if ball.x_vel < 0:
            return
    # 若为左机右人模式
    elif mode == 2:
        if ball.x_vel > 0:
            return

    # 生成随机因子，以便控制电脑难度
    r = random.randint(1, 100)

    # 如果为简单模式
    if level == 1:
        if r <= 80:
            if paddle.y + paddle.height // 2 < ball.y and 0 <= paddle.y + paddle.VEL <= HEIGHT - paddle.height:
                paddle.y += paddle.VEL
            elif paddle.y + paddle.height // 2 > ball.y and 0 <= paddle.y - paddle.VEL <= HEIGHT - paddle.height:
                paddle.y -= paddle.VEL

    # 如果为中等模式
    if level == 2:
        if r <= 90:
            if paddle.y + paddle.height // 2 < ball.y and 0 <= paddle.y + paddle.VEL <= HEIGHT - paddle.height:
                paddle.y += paddle.VEL
            elif paddle.y + paddle.height // 2 > ball.y and 0 <= paddle.y - paddle.VEL <= HEIGHT - paddle.height:
                paddle.y -= paddle.VEL

    # 如果为困难模式
    if level == 3:
        if r <= 98:
            if paddle.y + paddle.height // 2 < ball.y and 0 <= paddle.y + paddle.VEL <= HEIGHT - paddle.height:
                paddle.y += paddle.VEL
            elif paddle.y + paddle.height // 2 > ball.y and 0 <= paddle.y - paddle.VEL <= HEIGHT - paddle.height:
                paddle.y -= paddle.VEL


# 开始游戏
def play():
    run = True
    clock = pygame.time.Clock()

    # 创建左右球拍和乒乓球
    left_paddle = Paddle(10, HEIGHT // 2 - PADDLE_HEIGHT // 2,
                         PADDLE_WIDTH, PADDLE_HEIGHT)
    right_paddle = Paddle(WIDTH - 10 - PADDLE_WIDTH, HEIGHT // 2 - PADDLE_HEIGHT // 2,
                          PADDLE_WIDTH, PADDLE_HEIGHT)
    ball = Ball(WIDTH // 2, HEIGHT // 2, BALL_RADIUS)

    # 记录左右玩家得分和胜局数
    left_score = 0
    right_score = 0
    left_wins = 0
    right_wins = 0

    while run:
        # 设置刷新频率并绘制面板
        clock.tick(FPS)
        draw(SCREEN, [left_paddle, right_paddle], ball, left_score, right_score, left_wins, right_wins)

        # 判断是否关闭窗口
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()

        # 获取键盘动作，处理球拍移动
        keys = pygame.key.get_pressed()
        movement(keys, left_paddle, right_paddle, ball, MODE, LEVEL)

        # 处理乒乓球移动
        ball.move()

        # 处理碰撞
        collision(ball, left_paddle, right_paddle)

        # 处理得分
        if ball.x < 0:
            right_score += 1
            ball.reset()
        elif ball.x > WIDTH:
            left_score += 1
            ball.reset()

        # 处理小局输赢
        won, all_won = False, False
        if left_score >= WINNING_SCORE and left_score - right_score >= 2:
            won = True
            left_wins += 1
            win_text = "Left Player Won His Game Point!"
        elif right_score >= WINNING_SCORE and right_score - left_score >= 2:
            won = True
            right_wins += 1
            win_text = "Right Player Won His Game Point!"

        # 判断大盘获胜
        if left_wins == MAX_WIN:
            all_won = True
            final_win_text = "Left Player Won the Whole game!"
        elif right_wins == MAX_WIN:
            all_won = True
            final_win_text = "Right Player Won the Whole game!"

        # 输出胜负信息

        # 如果大盘胜利
        if all_won:
            text = get_font(30).render(final_win_text, True, "Red")
            SCREEN.blit(text, (WIDTH // 2 - text.get_width() // 2, HEIGHT // 2 - text.get_height() // 2))
            pygame.display.update()

            while True:
                # 获取鼠标位置
                NEW_MOUSE_POS = pygame.mouse.get_pos()

                # 设置新游戏、返回主菜单功能
                NEW_GAME = Button(image=None, pos=(640, 500), text_input="New game",
                                                 font=get_font(75), base_color="Blue", hovering_color="Green")
                BACK_TO_MAIN_MENU = Button(image=None, pos=(640, 600), text_input="Back",
                                           font=get_font(75), base_color="Blue", hovering_color="Green")

                # 设置鼠标选中变色效果
                NEW_GAME.changeColor(NEW_MOUSE_POS)
                NEW_GAME.update(SCREEN)
                BACK_TO_MAIN_MENU.changeColor(NEW_MOUSE_POS)
                BACK_TO_MAIN_MENU.update(SCREEN)

                # 判断鼠标选择的是哪个按钮
                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        pygame.quit()
                        sys.exit()
                    if event.type == pygame.MOUSEBUTTONDOWN:
                        if NEW_GAME.checkForInput(NEW_MOUSE_POS):
                            play()
                        elif BACK_TO_MAIN_MENU.checkForInput(NEW_MOUSE_POS):
                            main_menu()

                pygame.display.update()
            ball.reset()
            left_paddle.reset()
            right_paddle.reset()
            left_score = 0
            right_score = 0
            left_wins = 0
            right_wins = 0


        # 如果小盘胜利
        elif won:
            text = get_font(30).render(win_text, True, "Red")
            SCREEN.blit(text, (WIDTH // 2 - text.get_width() // 2, HEIGHT // 2 - text.get_height() // 2))
            pygame.display.update()

            flag = True
            while flag:
                # 获取鼠标位置
                NEW_MOUSE_POS = pygame.mouse.get_pos()

                # 设置新游戏、返回主菜单功能
                NEXT_GAME = Button(image=None, pos=(640, 500), text_input="Continue",
                                  font=get_font(75), base_color="Blue", hovering_color="Green")
                BACK_TO_MAIN_MENU = Button(image=None, pos=(640, 600), text_input="Back",
                                           font=get_font(75), base_color="Blue", hovering_color="Green")

                # 设置鼠标选中变色效果
                NEXT_GAME.changeColor(NEW_MOUSE_POS)
                NEXT_GAME.update(SCREEN)
                BACK_TO_MAIN_MENU.changeColor(NEW_MOUSE_POS)
                BACK_TO_MAIN_MENU.update(SCREEN)

                # 判断鼠标选择的是哪个按钮
                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        pygame.quit()
                        sys.exit()
                    if event.type == pygame.MOUSEBUTTONDOWN:
                        if NEXT_GAME.checkForInput(NEW_MOUSE_POS):
                            flag = False
                        elif BACK_TO_MAIN_MENU.checkForInput(NEW_MOUSE_POS):
                            main_menu()

                pygame.display.update()
            ball.reset()
            left_paddle.reset()
            right_paddle.reset()
            left_score = 0
            right_score = 0


# 设置游戏难度
def set_difficulty():
    while True:
        # 获得鼠标
        SET_DIFFICULTY_MOUSE_POS = pygame.mouse.get_pos()

        # 填充背景
        SCREEN.fill(WHITE)

        # 设置提示文字
        SET_DIFFICULTY_TEXT = get_font(80).render("GAME DIFFICULTY", True, "Red")
        OPTIONS_RECT = SET_DIFFICULTY_TEXT.get_rect(center=(640, 100))
        SCREEN.blit(SET_DIFFICULTY_TEXT, OPTIONS_RECT)

        # 设置简单、中等、困难三个难度，并设置返回键
        SET_DIFFICULTY_EASY = Button(image=None, pos=(640, 250), text_input="Easy",
                              font=get_font(75), base_color="Blue", hovering_color="Green")
        SET_DIFFICULTY_MIDDLE = Button(image=None, pos=(640, 350), text_input="Middle",
                                font=get_font(75), base_color="Blue", hovering_color="Green")
        SET_DIFFICULTY_HARD = Button(image=None, pos=(640, 450), text_input="Hard",
                              font=get_font(75), base_color="Blue", hovering_color="Green")
        SET_DIFFICULTY_BACK = Button(image=None, pos=(640, 550), text_input="Back",
                              font=get_font(75), base_color="Black", hovering_color="Green")

        # 设置鼠标选中变色效果
        SET_DIFFICULTY_EASY.changeColor(SET_DIFFICULTY_MOUSE_POS)
        SET_DIFFICULTY_EASY.update(SCREEN)
        SET_DIFFICULTY_MIDDLE.changeColor(SET_DIFFICULTY_MOUSE_POS)
        SET_DIFFICULTY_MIDDLE.update(SCREEN)
        SET_DIFFICULTY_HARD.changeColor(SET_DIFFICULTY_MOUSE_POS)
        SET_DIFFICULTY_HARD.update(SCREEN)
        SET_DIFFICULTY_BACK.changeColor(SET_DIFFICULTY_MOUSE_POS)
        SET_DIFFICULTY_BACK.update(SCREEN)

        # 判断鼠标选择的是哪个按钮
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.MOUSEBUTTONDOWN:
                if SET_DIFFICULTY_EASY.checkForInput(SET_DIFFICULTY_MOUSE_POS):
                    # 简单模式:LEVEL = 1
                    global  LEVEL
                    LEVEL = 1
                    settings()
                elif SET_DIFFICULTY_MIDDLE.checkForInput(SET_DIFFICULTY_MOUSE_POS):
                    # 中等模式:LEVEL = 2
                    LEVEL = 2
                    settings()
                elif SET_DIFFICULTY_HARD.checkForInput(SET_DIFFICULTY_MOUSE_POS):
                    # 困难模式:LEVEL = 3
                    LEVEL = 3
                    settings()
                elif SET_DIFFICULTY_BACK.checkForInput(SET_DIFFICULTY_MOUSE_POS):
                    settings()

        pygame.display.update()


# 设置游戏局数
def set_games_to_win():
    while True:
        # 获取鼠标位置
        SET_GAMES_TO_WIN_MOUSE_POS = pygame.mouse.get_pos()

        # 填充背景
        SCREEN.fill("White")

        # 设置提示文字
        SET_GAMES_TO_WIN_TEXT = get_font(50).render("GAMES TO WIN", True, "RED")
        SET_GAMES_TO_WIN_RECT = SET_GAMES_TO_WIN_TEXT.get_rect(center=(640, 100))
        SCREEN.blit(SET_GAMES_TO_WIN_TEXT, SET_GAMES_TO_WIN_RECT)

        # 设置一局定胜负、三局两胜、五局三胜、七局四胜四种游戏模式，并设置返回键
        SET_GAME_TO_WIN_M1 = Button(image=None, pos=(640, 250), text_input="The best of one",
                                     font=get_font(40), base_color="Blue", hovering_color="Green")
        SET_GAME_TO_WIN_M2 = Button(image=None, pos=(640, 350), text_input="The best of three",
                                       font=get_font(40), base_color="Blue", hovering_color="Green")
        SET_GAME_TO_WIN_M3 = Button(image=None, pos=(640, 450), text_input="The best of five",
                                     font=get_font(40), base_color="Blue", hovering_color="Green")
        SET_GAME_TO_WIN_M4 = Button(image=None, pos=(640, 550), text_input="The best of seven",
                                   font=get_font(40), base_color="Blue", hovering_color="Green")
        SET_GAME_TO_WIN_BACK = Button(image=None, pos=(640, 650), text_input="Back",
                                     font=get_font(40), base_color="Black", hovering_color="Green")

        # 设置鼠标选中变色效果
        SET_GAME_TO_WIN_M1.changeColor(SET_GAMES_TO_WIN_MOUSE_POS)
        SET_GAME_TO_WIN_M1.update(SCREEN)
        SET_GAME_TO_WIN_M2.changeColor(SET_GAMES_TO_WIN_MOUSE_POS)
        SET_GAME_TO_WIN_M2.update(SCREEN)
        SET_GAME_TO_WIN_M3.changeColor(SET_GAMES_TO_WIN_MOUSE_POS)
        SET_GAME_TO_WIN_M3.update(SCREEN)
        SET_GAME_TO_WIN_M4.changeColor(SET_GAMES_TO_WIN_MOUSE_POS)
        SET_GAME_TO_WIN_M4.update(SCREEN)
        SET_GAME_TO_WIN_BACK.changeColor(SET_GAMES_TO_WIN_MOUSE_POS)
        SET_GAME_TO_WIN_BACK.update(SCREEN)

        # 判断鼠标选择的是哪个按钮
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.MOUSEBUTTONDOWN:
                if SET_GAME_TO_WIN_M1.checkForInput(SET_GAMES_TO_WIN_MOUSE_POS):
                    # 一局定胜负模式:MAX_GAME = 1, MAX_WIN = 1
                    global MAX_GAME
                    global MAX_WIN
                    MAX_GAME = 1
                    MAX_WIN = 1
                    set_mode()
                elif SET_GAME_TO_WIN_M2.checkForInput(SET_GAMES_TO_WIN_MOUSE_POS):
                    # 三局两胜模式:MAX_GAME = 3, MAX_WIN = 2
                    MAX_GAME = 3
                    MAX_WIN = 2
                    set_mode()
                elif SET_GAME_TO_WIN_M3.checkForInput(SET_GAMES_TO_WIN_MOUSE_POS):
                    # 五局三胜模式:MAX_GAME = 5, MAX_WIN = 3
                    MAX_GAME = 5
                    MAX_WIN = 3
                    set_mode()
                elif SET_GAME_TO_WIN_M4.checkForInput(SET_GAMES_TO_WIN_MOUSE_POS):
                    # 七局四胜模式:MAX_GAME = 7, MAX_WIN = 4
                    MAX_GAME = 7
                    MAX_WIN = 4
                    set_mode()
                elif SET_GAME_TO_WIN_BACK.checkForInput(SET_GAMES_TO_WIN_MOUSE_POS):
                    set_mode()

        pygame.display.update()


# 设置对战模式
def set_competition_mode():
    while True:
        # 获取鼠标位置
        SET_COMPETITION_MODE_MOUSE_POS = pygame.mouse.get_pos()

        # 填充背景
        SCREEN.fill("White")

        # 设置提示文字
        SET_COMPETITION_MODE_TEXT = get_font(50).render("COMPETITION MODE", True, "RED")
        SET_COMPETITION_MODE_RECT = SET_COMPETITION_MODE_TEXT.get_rect(center=(640, 100))
        SCREEN.blit(SET_COMPETITION_MODE_TEXT, SET_COMPETITION_MODE_RECT)

        # 设置左人右机、左机右人、玩家对战三种游戏模式，并设置返回键
        SET_COMPETITION_MODE_M1 = Button(image=None, pos=(640, 250), text_input="Left: player Right: computer",
                             font=get_font(40), base_color="Blue", hovering_color="Green")
        SET_COMPETITION_MODE_M2 = Button(image=None, pos=(640, 350), text_input="Left: computer Right: player",
                                         font=get_font(40), base_color="Blue", hovering_color="Green")
        SET_COMPETITION_MODE_M3 = Button(image=None, pos=(640, 450), text_input="Left: player1 Right: player2",
                                         font=get_font(40), base_color="Blue", hovering_color="Green")
        SET_COMPETITION_MODE_BACK = Button(image=None, pos=(640, 550), text_input="Back",
                               font=get_font(40), base_color="Black", hovering_color="Green")

        # 设置鼠标选中变色效果
        SET_COMPETITION_MODE_M1.changeColor(SET_COMPETITION_MODE_MOUSE_POS)
        SET_COMPETITION_MODE_M1.update(SCREEN)
        SET_COMPETITION_MODE_M2.changeColor(SET_COMPETITION_MODE_MOUSE_POS)
        SET_COMPETITION_MODE_M2.update(SCREEN)
        SET_COMPETITION_MODE_M3.changeColor(SET_COMPETITION_MODE_MOUSE_POS)
        SET_COMPETITION_MODE_M3.update(SCREEN)
        SET_COMPETITION_MODE_BACK.changeColor(SET_COMPETITION_MODE_MOUSE_POS)
        SET_COMPETITION_MODE_BACK.update(SCREEN)

        # 判断鼠标选择的是哪个按钮
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.MOUSEBUTTONDOWN:
                if SET_COMPETITION_MODE_M1.checkForInput(SET_COMPETITION_MODE_MOUSE_POS):
                    # 左人右机模式:MODE = 1
                    global MODE
                    MODE = 1
                    set_mode()
                elif SET_COMPETITION_MODE_M2.checkForInput(SET_COMPETITION_MODE_MOUSE_POS):
                    # 左机右人模式:MODE = 2
                    MODE = 2
                    set_mode()
                elif SET_COMPETITION_MODE_M3.checkForInput(SET_COMPETITION_MODE_MOUSE_POS):
                    # 玩家对战模式:MODE = 3
                    MODE = 3
                    set_mode()
                elif SET_COMPETITION_MODE_BACK.checkForInput(SET_COMPETITION_MODE_MOUSE_POS):
                    set_mode()

        pygame.display.update()


# 设置游戏模式
def set_mode():
    while True:
        # 获取鼠标位置
        SET_MODE_MOUSE_POS = pygame.mouse.get_pos()

        # 填充背景
        SCREEN.fill("White")

        # 设置提示文字
        SET_MODE_TEXT = get_font(50).render("GAME MODE", True, "RED")
        SET_MODE_RECT = SET_MODE_TEXT.get_rect(center=(640, 100))
        SCREEN.blit(SET_MODE_TEXT, SET_MODE_RECT)

        # 设置对战模式、游戏局数，并设置返回键
        SET_MODE_SET_COMPETITION_MODE = Button(image=None, pos=(640, 250), text_input="Set competition mode",
                                         font=get_font(40), base_color="Blue", hovering_color="Green")
        SET_MODE_SET_GAMES_TO_WIN = Button(image=None, pos=(640, 350), text_input="Set games to win",
                                   font=get_font(40), base_color="Blue", hovering_color="Green")
        SET_MODE_BACK = Button(image=None, pos=(640, 450), text_input="Back",
                               font=get_font(40), base_color="Black", hovering_color="Green")

        # 设置鼠标选中变色效果
        SET_MODE_SET_COMPETITION_MODE.changeColor(SET_MODE_MOUSE_POS)
        SET_MODE_SET_COMPETITION_MODE.update(SCREEN)
        SET_MODE_SET_GAMES_TO_WIN.changeColor(SET_MODE_MOUSE_POS)
        SET_MODE_SET_GAMES_TO_WIN.update(SCREEN)
        SET_MODE_BACK.changeColor(SET_MODE_MOUSE_POS)
        SET_MODE_BACK.update(SCREEN)

        # 判断鼠标选择的是哪个按钮
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.MOUSEBUTTONDOWN:
                if SET_MODE_SET_COMPETITION_MODE.checkForInput(SET_MODE_MOUSE_POS):
                    set_competition_mode()
                elif SET_MODE_SET_GAMES_TO_WIN.checkForInput(SET_MODE_MOUSE_POS):
                    set_games_to_win()
                elif SET_MODE_BACK.checkForInput(SET_MODE_MOUSE_POS):
                    settings()

        pygame.display.update()


# 设置选项
def settings():
    while True:
        # 获取鼠标位置
        SETTINGS_MOUSE_POS = pygame.mouse.get_pos()

        # 填充背景
        SCREEN.fill("white")

        # 设置提示文字
        SETTINGS_TEXT = get_font(80).render("SETTINGS", True, "RED")
        SETTINGS_RECT = SETTINGS_TEXT.get_rect(center=(640, 100))
        SCREEN.blit(SETTINGS_TEXT, SETTINGS_RECT)

        # 设置游戏难度、游戏模式，并设置返回键
        SETTINGS_SET_DIFFICULTY = Button(image=None, pos=(640, 250), text_input="Set difficulty",
                              font=get_font(75), base_color="Black", hovering_color="Green")
        SETTINGS_SET_MODE = Button(image=None, pos=(640, 350), text_input="Set mode",
                              font=get_font(75), base_color="Black", hovering_color="Green")
        SETTINGS_BACK = Button(image=None, pos=(640, 450), text_input="Back",
                              font=get_font(75), base_color="Black", hovering_color="Green")

        # 设置鼠标选中变色效果
        SETTINGS_SET_DIFFICULTY.changeColor(SETTINGS_MOUSE_POS)
        SETTINGS_SET_DIFFICULTY.update(SCREEN)
        SETTINGS_SET_MODE.changeColor(SETTINGS_MOUSE_POS)
        SETTINGS_SET_MODE.update(SCREEN)
        SETTINGS_BACK.changeColor(SETTINGS_MOUSE_POS)
        SETTINGS_BACK.update(SCREEN)

        # 判断鼠标选择的是哪个按钮
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.MOUSEBUTTONDOWN:
                if SETTINGS_SET_DIFFICULTY.checkForInput(SETTINGS_MOUSE_POS):
                    set_difficulty()
                elif SETTINGS_SET_MODE.checkForInput(SETTINGS_MOUSE_POS):
                    set_mode()
                elif SETTINGS_BACK.checkForInput(SETTINGS_MOUSE_POS):
                    main_menu()


        pygame.display.update()


# 主菜单
def main_menu():
    while True:
        # 填充背景
        SCREEN.blit(BG, (0, 0))

        # 获取鼠标位置
        MENU_MOUSE_POS = pygame.mouse.get_pos()

        # 设置提示文字
        MENU_TEXT = get_font(75).render("Ping-pong genius", True, "#b68f40")
        MENU_RECT = MENU_TEXT.get_rect(center=(640, 100))
        SCREEN.blit(MENU_TEXT, MENU_RECT)

        # 设置开始、设置、退出三个按钮
        PLAY_BUTTON = Button(image=pygame.image.load("Play Rect.png"), pos=(640, 250), text_input="PLAY",
                             font=get_font(75), base_color="#d7fcd4", hovering_color="White")
        SETTINGS_BUTTON = Button(image=pygame.image.load("Setting Rect.png"), pos=(640, 400), text_input="SETTING",
                                font=get_font(75), base_color="#d7fcd4", hovering_color="White")
        QUIT_BUTTON = Button(image=pygame.image.load("Quit Rect.png"), pos=(640, 550), text_input="QUIT",
                             font=get_font(75), base_color="#d7fcd4", hovering_color="White")

        # 设置鼠标选中变色效果
        for button in [PLAY_BUTTON, SETTINGS_BUTTON, QUIT_BUTTON]:
            button.changeColor(MENU_MOUSE_POS)
            button.update(SCREEN)

        # 判断鼠标选择的是哪个按钮
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.MOUSEBUTTONDOWN:
                if PLAY_BUTTON.checkForInput(MENU_MOUSE_POS):
                    play()
                if SETTINGS_BUTTON.checkForInput(MENU_MOUSE_POS):
                    settings()
                if QUIT_BUTTON.checkForInput(MENU_MOUSE_POS):
                    pygame.quit()
                    sys.exit()

        pygame.display.update()


if __name__ == '__main__':
    main_menu()
