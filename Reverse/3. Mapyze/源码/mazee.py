import time
from turtle import Turtle
import turtle
import string

#设置游戏的窗口大小和背景颜色
turtle.screensize(300,200, "pink")

class Maze(Turtle):
    size = 20    #迷宫内一格墙的长宽
    
    def __init__(self, maze_list):
      # 需要先调用父类的初始化方法才能在初始化方法中调用父类的方法
      Turtle.__init__(self)
      self.maze_list = maze_list
      # 为了加快绘图速度隐藏海龟，速度设为最快
      self.hideturtle()
      self.speed(0)
      self.draw_walls()

  #绘制迷宫内一格墙的过程
    def draw_wall(self):
      self.pendown()
      self.begin_fill()
      #绘制墙的颜色
      self.fillcolor('red')
      #首先画一个距离为20的横线，再向右旋转90度，循环4次形成方形
      for i in range(4):
        self.forward(self.size)
        self.right(90)
      self.end_fill()
      self.penup()

  #绘制整个迷宫的墙
    def draw_walls(self):
      self.penup()
      # 从 (-130, 130) 开始
      self.goto(-130, 130)
      #打印墙，横纵循环13次（整个迷宫的长和宽由13格墙组成）
      for row in range(13):
        for col in range(13):
          #主函数中的maze_list里面的1则打印出一格墙
          if self.maze_list[row][col] == 1:
            self.draw_wall()
          # 右移一列
          self.goto(self.size * (col + 1) - 130, 130 - self.size * row)
        # 下移一行
        self.goto(-130, 130 - self.size * (row + 1))


class Player(Turtle):
    def __init__(self, maze_list, start_m, start_n, end_m, end_n):
      # 父类初始化
      Turtle.__init__(self)
      #初始的横纵坐标
      self.m = start_m
      self.n = start_n
      #终点的横纵坐标
      self.end_m = end_m
      self.end_n = end_n
      #迷宫地图
      self.maze_list = maze_list
      self.hideturtle()
      self.speed(0)
      self.penup()
      # 玩家移到对应的位置
      self.goto(self.n * 20 - 120, 120 - self.m * 20)
      # 生成玩家
      self.shape('turtle')
      self.color('pink')
      #玩家初始方向
      self.setheading(270)
      self.showturtle()
      
  #当玩家到达终点时，显示'you win!'
    def reach_exit(self, m, n):
      if m == self.end_m and n == self.end_n:
        # 走出迷宫，显示'you win!'
        text = turtle.Turtle()
        text.hideturtle()
        text.penup()
        text.goto(-125, -10)
        text.color('blue')
        text.write('flag{you_', font = ('SimHei', 48, 'bold'))

    #定义玩家可移动的位置，即只允许在迷宫内的通道里移动
    def canmove(self, m, n):
      #遇到0允许移动
      try:
          return self.maze_list[m][n] == 0
      except:
          return 0
    #玩家移动时位置发生的变化
    def move(self, m, n):
      self.m = m
      self.n = n
      self.goto(self.n * 20 - 120, 120 - self.m * 20)
      self.reach_exit(m, n)

    #向上移动
    def go_up(self):
      if self.canmove(self.m - 1, self.n):
        self.setheading(90)
        self.move(self.m - 1, self.n)


    #向下移动
    def go_down(self):
      if self.canmove(self.m + 1, self.n):
        self.setheading(270)
        self.move(self.m + 1, self.n)


    #向左移动
    def go_left(self):
      if self.canmove(self.m, self.n - 1):
        self.setheading(180)
        self.move(self.m, self.n - 1)


    #向右移动
    def go_right(self):
      if self.canmove(self.m, self.n + 1):
        self.setheading(0)
        self.move(self.m, self.n + 1)


def bencode(inputs):
    s ='abcefd' + string.ascii_letters[:5:-1] + string.digits + '+/'
    # 将字符串转化为2进制
    bin_str = []
    for i in inputs:
      x = str(bin(ord(i))).replace('0b', '')
      bin_str.append('{:0>8}'.format(x))
    #print(bin_str)
    # 输出的字符串
    outputs = ""
    # 不够三倍数，需补齐的次数
    nums = 0
    while bin_str:
      #每次取三个字符的二进制
      temp_list = bin_str[:3]
      if(len(temp_list) != 3):
        nums = 3 - len(temp_list)
        while len(temp_list) < 3:
          temp_list += ['0' * 8]
      temp_str = "".join(temp_list)
      #print(temp_str)
      # 将三个8字节的二进制转换为4个十进制
      temp_str_list = []
      for i in range(0,4):
        temp_str_list.append(int(temp_str[i*6:(i+1)*6],2))
      #print(temp_str_list)
      if nums:
        temp_str_list = temp_str_list[0:4 - nums]
        
      for i in temp_str_list:
        outputs += s[i]
      bin_str = bin_str[3:]
    outputs += nums * '='
    return outputs

if __name__ == '__main__':
    print('          Dont close me!!!\nFind the way out and get the first part flag\n     As for the rest, guess!  :)')
    maze_list = [
    [1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1],
    [1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1],
    [1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1],
    [1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1],
    [1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1],
    [1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1],
    [1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1],
    [1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1],
    [1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1],
    [1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1],
    [1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1],
    [1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1],
    [1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1]
    ]

    Maze(maze_list)
    #0,5表示玩家起始的位置；12,7表示终点的位置
    player = Player(maze_list, 0, 5, 12, 7)
    check_flag = 1 
    while check_flag == 1:
        sc = turtle.Screen()

        turtle.onkey(player.go_down,'Down')
        turtle.onkey(player.go_up,'Up')
        turtle.onkey(player.go_left,'Left')
        turtle.onkey(player.go_right,'Right')
        sc.listen()

        flag = 'flag{you_'
        rest = input('Just give it to me [doge]:')
        if bencode(flag + rest) != 'GtiyG3m5E3KAHIWuI2dACYW1GK9wCZGuDs0=':
            print('you are wrong!')
            continue
        else:
            print('you are right!')
            time.sleep(1)
            check_flag = 0  
    exit(0)