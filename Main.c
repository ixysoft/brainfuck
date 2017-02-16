/*
 *	brainfuck解释器:
 *	基础语法:
 *		+ 当前位置内容 +1
 *		- 当前位置内容 -1
 *		< 当前位置指针左移
 *		> 当前位置指针右移
 *		[ 循环开始符
 *		] 循环结尾符
 *		, 输入
 *		. 输出
 *	扩展语法:
 *		& 将当前指针的位置存入当前的当前位置
 *		$ 输入字符串
 *		~ 将当前位置内容取反
 *		(简单的命令)
 *			+n 将当前位置的值+n
 *			-n 将当前位置的值-n
 *			*n 将当前位置的值×n
 *			/n 将当前位置的值/n
 *			%n 将当前位置的值%n
 *			=n 将当前位置的值置为n
 *			^n 将当前位置的值^n
 *			@指令名 调用指令
 *		  &指针模式
 *		  *buf模式(没有具体实现)
 *		? 输出当前状态
 *		{指令名:命令内容} 定义指令
 *
 *		mode的说明(未应用)
 *		0x00 初始模式
 *		0x01 循环模式
 *		0x02 指令模式
 *		0x04 过程模式
 * */

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define MODE_INIT 0x00000000
#define MODE_LOOP 0x00000001
#define MODE_CODE 0x00000002
#define MODE_PROC 0x00000004

//自定义常量
#define MAX_POS 8092
#define MAX_BUF_POS 8092
#define CMD_LEN 8092

typedef char * string;
typedef struct command{
	char name[256];//名称
	char cmd[CMD_LEN];//命令
	struct command *next;
}*command,Command;

int mode=0x00;//储存模式
unsigned char storage[MAX_POS];//内存空间
int	pos=0;//指针位置
int max_pos=0;//最大的位置指针
unsigned char buffer[MAX_BUF_POS];//储存缓存变量
int buf_pos=0;//缓存指针的值

Command cmd_link;//全局的命令列表

int addCommand(string name,string cmd){//向命令列表中添加一条命令
	command tmp=&cmd_link;
	for(;tmp->next!=NULL;tmp=tmp->next){
		if(strcmp(tmp->next->name,name)==0){
			strcpy(tmp->next->cmd,cmd);
			return 0;
		}
	}
	command new_cmd=(command)malloc(sizeof(Command));
	if(new_cmd==NULL) return 1;
	new_cmd->next=cmd_link.next;
	strcpy(new_cmd->name,name);
	strcpy(new_cmd->cmd,cmd);
	cmd_link.next=new_cmd;
	return 0;
}

string proc(string name){//获取过程值
	command head=&cmd_link;
	for(;head->next;head=head->next){
		if(strcmp(head->next->name,name)==0){
			return head->next->cmd;
		}
	}
	return "";
}

void show(){
	command head=&cmd_link;
	for(;head->next;head=head->next){
		printf("%s:%s\n",head->next->name,head->next->cmd);
	}
}

int brainfuck(string cmd){//解析命令
	//字符命令
	if(strcmp(cmd,"")==0){
		return 0;
	}else if(strcmp(cmd,"quit")==0){
		exit(0);
	}else if(strcmp(cmd,"reset")==0){//初始化
		memset(storage,0,max_pos);
		max_pos=pos=buf_pos=0;
		command tmp;
		for(tmp=cmd_link.next;tmp!=NULL;tmp=cmd_link.next){
			cmd_link.next=cmd_link.next->next;
			free(tmp);
		}
	}else if(strcmp(cmd,"help")==0){//帮助
		printf("brainfuck帮助:\n");
		printf("基础语法:\n");
		printf("+		当前指针内容+1\n");
		printf("-		当前指针内容-1\n");
		printf("<		当前指针左移\n");
		printf(">		当前指针右移\n");
		printf("[		循环开始部分,相当于\"while(*ptr){\"	,ptr为当前指针\n");
		printf("]		循环结尾部分，相当于\"}\"\n");
		printf(",		输入\n");
		printf(".		输出\n");
		printf("扩展语法:\n");
		printf("(+n)		当前内存值+n\n");
		printf("(-n)		当前内存值-n\n");
		printf("(*n)		当前内存值*n\n");
		printf("(/n)		当前内存值/n\n");
		printf("(%%n)		当前内存值%%n\n");
		printf("(^n)		当前内存值^n\n");
		printf("(&n)		当前内存值&n\n");
		printf("(|n)		当前内存值|n\n");
		printf("~		当前内存值取反\n");
		printf("$		读取一个字符串\n");
		printf("&		将当前指针值存入内存\n");
		printf("?		读取状态信息");
		printf("(@过程名)	调用一个过程\n");
		printf("{过程名:具体实现}	定义一个过程\n");
		printf("文字命令:\n");
		printf("help		查看帮助信息\n");
		printf("cls		清屏\n");
		printf("reset		系统初始化\n");
		printf("quit		退出\n\n");
	}else if(strcmp(cmd,"cls")==0){
		system("printf \"\\033c\"");
	}else{
		int i,j,k;
		char ch;//当前字符
		char code[256];//自定义指令数组
		int tmp;
		for(i=0;(ch=cmd[i]) != '\0';i++){//遍历命令字符串
			//printf("pos=%d[%d]",pos,storage[pos]);
			//getchar();
			switch(ch){
				//基本命令
				case '+'://值+1
					if(mode & MODE_CODE){//指令模式
						buffer[buf_pos]++;//buffer内容+1
					}else{
						storage[pos]++;//内存内容+1
					}
					break;
				case '-'://值-1
					if(mode & MODE_CODE){
						buffer[buf_pos]--;
					}else{
						storage[pos]--;
					}
					break;
				case '<'://指针左移
					if(mode & MODE_CODE){
						if(buf_pos > 0){
							buf_pos--;
						}
					}else{
						if(pos > 0){
							pos--;
						}
					}
					break;
				case '>'://指针右移
					if(mode & MODE_CODE){
						if(buf_pos+1 < MAX_BUF_POS){
							buf_pos++;
						}
					}else{
						if(pos+1 < MAX_POS){
							pos++;
							max_pos=max_pos<pos?pos:max_pos;
						}
					}
					break;
				case '['://循环标志开始
					tmp=1;
					if(storage[pos]==0){
						for(i++;tmp>0 && cmd[i]!='\0';i++){
							if(cmd[i]=='[') tmp++;
							else if(cmd[i]==']') tmp--;
						}
					}
					break;
				case ']'://循环标志末尾
					tmp=1;//保存循环深度
					if(storage[pos]>0){//当前的值>0
						for(i--;i>0&& tmp>0;i--){
							if(cmd[i]==']'){
								tmp++;//循环深度+1
							}else if(cmd[i]=='['){
								tmp--;//循环深度-1
							}
						}
						//i--;
					}
					break;
				case ','://输入
					storage[pos]=fgetc(stdin);
					while(getchar()!='\n');
					break;
				case '.'://输出
					putchar(storage[pos]);
					break;
				//扩展语法
				case '~'://取反
					storage[pos]=~storage[pos];
					break;
				case '&':
					storage[pos]=pos;
					break;
				case '('://指令模式
					//mode=MODE_CODE;
					for(i++,j=0;cmd[i]!=')';i++,j++)
						code[j]=cmd[i];
					code[j]='\0';
					if(j<2) break;
					//检测数据的合法性
					if(code[0]!='@'){
						for(k=1;k<j;k++){
							if(code[k]<'0'||code[k]>'9') break;
						}
					}
					switch(code[0]){
						case '+':
							storage[pos]+=(unsigned char)atoi(code+1);
							break;
						case '-':
							storage[pos]-=(unsigned char)atoi(code+1);
							break;
						case '*':
							storage[pos]*=(unsigned char)atoi(code+1);
							break;
						case '/':
							if(atoi(code+1))storage[pos]/=(unsigned char)atoi(code+1);
							else printf("除0错误!\n");
							break;
						case '%':
							if(atoi(code+1)) storage[pos]+=(unsigned char)atoi(code+1);
							else printf("除0错误!\n");
							break;
						case '^':
							storage[pos]^=(unsigned char)atoi(code+1);
							break;
						case '&':
							storage[pos]&=(unsigned char)atoi(code+1);
							break;
						case '|':
							storage[pos]|=(unsigned char)atoi(code+1);
							break;
						case '=':
							storage[pos]=(unsigned char)atoi(code+1);
							break;
						case '@'://指令模式
							//printf("对应命令%s\n",proc(code+1));
							brainfuck(proc(code+1));//执行指令
							//show();
							break;
					}
					break;
				case ')':
					break;
				case '{':
					j=-1;
					for(i++,k=0;cmd[i]!='}' && cmd[i]!='\0';i++,k++){
						code[k]=cmd[i];//获取中间的代码，如{test:++++++}
						if(cmd[i]==':'){
							code[k]='\0';
							j=k;
						}
					}
					code[k]='\0';
					addCommand(code,code+j+1);//增加一条命令
					break;
				case '}':
					break;
				case '?'://帮助
					printf("状态信息:\n");
					for(j=0;j<=max_pos;j++){
						printf("[ storage[%d]=%d -- %c ]",
								j,storage[j],storage[j]);
						if(j==pos) printf("\t<==当前位置");
						puts("");
					}
					break;
				case '$'://输入字符串
					scanf("%s",&storage[pos]);
					getchar();
					break;
			}
		}
	}
}

void end_code(){
	puts("谢谢使用brainfuck系统!");
}

int main(int argc,char *argv){
	atexit(end_code);
	string cmd=(string)malloc(sizeof(CMD_LEN));
	char ch;
	int i;
	//初始化命令链表
	cmd_link.next=NULL;
	addCommand("test","(=65).>(=10).");
	if(argc<2){//没有任何选项
		for(;1;){
			printf("brainfuck: ");
			for(i=0;(ch=getchar())!='\n';i++){
				cmd[i]=ch;
			}
			cmd[i]='\0';
			brainfuck(cmd);
		}
	}else{
		
	}
}