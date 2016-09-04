
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

int strcmp(char *str1,char *str2)
{
	int i;
	for (i=0; i<strlen(str1); i++)
	{
		if (i==strlen(str2)) return 1;
		if (str1[i]>str2[i]) return 1;
		else if (str1[i]<str2[i]) return -1;
	}
	return 0;
}

void strlwr(char *str)
{
	int i;
	for (i=0; i<strlen(str); i++)
	{
		if ('A'<=str[i] && str[i]<='Z') str[i]=str[i]+'a'-'A';
	}
}

void addToQueue(PROCESS* p)
{
	p->state=kRUNNABLE;
	if (p->priority>=10)
	{
		firstQueue[firstLen]=p;
		firstLen++;
		p->ticks=2;
		p->whichQueue=1;
	}
	else if (p->priority>=5)
	{
		secondQueue[secondLen]=p;
		secondLen++;
		p->ticks=3;
		p->whichQueue=2;
	}
	else
	{
		thirdQueue[thirdLen]=p;
		thirdLen++;
		p->ticks=p->priority;
		p->whichQueue=3;
	}
}

/*======================================================================*
                            linx_main
 *======================================================================*/
PUBLIC int linx_main()
{
	
	clearScreen();
	
	milli_delay(3);
	DisPlayAnimation();
	displayWelcome();

	TASK*		p_task;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	t_16		selector_ldt	= SELECTOR_LDT_FIRST;
	int		i;
	t_8		privilege;
	t_8		rpl;
	int		eflags;
	for(i=0;i<NR_TASKS+NR_PROCS;i++){
		if (i < NR_TASKS) {	/* 任务 */
			p_task		= task_table + i;
			privilege	= PRIVILEGE_TASK;
			rpl		= RPL_TASK;
			eflags		= 0x1202;	/* IF=1, IOPL=1, bit 2 is always 1 */
		}
		else {			/* 用户进程 */
			p_task		= user_proc_table + (i - NR_TASKS);
			privilege	= PRIVILEGE_USER;
			rpl		= RPL_USER;
			eflags		= 0x202;	/* IF=1, bit 2 is always 1 */
		}

		strcpy(p_proc->name, p_task->name);	/* name of the process */
		p_proc->pid	= i;			/* pid */

		p_proc->ldt_sel	= selector_ldt;
		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;	/* change the DPL */
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;/* change the DPL */
		p_proc->regs.cs		= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs		= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p_proc->regs.eip	= (t_32)p_task->initial_eip;
		p_proc->regs.esp	= (t_32)p_task_stack;
		p_proc->regs.eflags	= eflags;

		p_proc->nr_tty		= 0;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	//修改这里的优先级和ticks
	proc_table[0].priority = 15;
	proc_table[1].priority =  5;
	proc_table[2].priority =  2;
	proc_table[3].priority =  5;
	proc_table[4].priority =  2;
	proc_table[5].priority =  10;
	proc_table[6].priority =  20;
	proc_table[7].priority =  10;
	proc_table[8].priority = 10;
	proc_table[9].priority = 10;

	//对优先队列初始化
	thirdLen=0;
	firstLen=firstHead=secondLen=0;
	for (i=0; i<NR_TASKS+NR_PROCS;i++)
	{
		addToQueue(proc_table+i);
	}
	//指定控制台
	proc_table[1].nr_tty = 0;
	proc_table[2].nr_tty = 1;
	proc_table[3].nr_tty = 1;
	proc_table[4].nr_tty = 1;
	proc_table[5].nr_tty = 1;
	proc_table[6].nr_tty = 2;
	proc_table[7].nr_tty = 3;
	proc_table[8].nr_tty = 4;
	proc_table[9].nr_tty = 5;

	k_reenter	= 0;
	ticks		= 0;

	p_proc_ready	= proc_table;

	init_clock();

	restart();

	while(1){}
}

/*======================================================================*
                            clearScreen
 *======================================================================*/

void clearScreen()
{
	int i;
	disp_pos=0;
	for(i=0;i<80*25;i++)
	{
		disp_str(" ");
	}
	disp_pos=0;
	
}

/*======================================================================*
                            help
 *======================================================================*/

void help()
{
	printf("      ==========================================================\n");
	printf("      =====  help         --------  show the help menu     =====\n");
	printf("      =====  clear        --------  clear screen           =====\n");	
	printf("      =====  show         --------  show the process info  =====\n");
	printf("      =====  shutdown     --------  shut down the computer =====\n");
	printf("      =====  F1           --------  return main menu       =====\n");
	printf("      =====  F2           --------  show the process run   =====\n");
	printf("      =====  F3           --------  calculator             =====\n");
	printf("      =====  F4           --------  Game                   =====\n");
	printf("      =====  F5           --------  calendar               =====\n");
	printf("      =====  F6           --------  task manager           =====\n");
	printf("      ==========================================================\n");
	printf("\n");
}

/*======================================================================*
                            show process
 *======================================================================*/

void show()
{
	PROCESS* p;
	int i;
	printf("==========================================================================\n");	
	printf("procs_ID      proc_name      proc_priority      queque_ID      procs_state\n");
	printf("--------------------------------------------------------------------------\n");
	for (i=0; i<NR_TASKS+NR_PROCS;i++)
	{
		p=&proc_table[i];
		printf("process%d:      ",p->pid);
		int len = 12;
		printf("%s",p->name);
		len = len-strlen(p->name)+5;
		while(len--)
			printf(" ");
		printf("%d                ",p->priority);
		if(p->priority<10) printf(" ");
		printf("%d            ",p->whichQueue);
		
		switch (p->state)
		{
		case kRUNNABLE:
			printf("Runnable\n");
			break;
		case kRUNNING:
			printf("Running\n");
			break;
		case kREADY:
			printf("Ready\n");
			break;
		}
	}
}

/*======================================================================*
                            readOneStringAndOneNumber
 *======================================================================*/

void readOneStringAndOneNumber(char* command,char* str,int* number)
{
	int i;
	int j=0;
	for (i=0; i<strlen(command); i++)
	{
		if (command[i]!=' ') break;
	}
	for (; i<strlen(command); i++)
	{
		if (command[i]==' ') break;
		str[j]=command[i];
		j++;
	}
	for (; i<strlen(command); i++)
	{
		if (command[i]!=' ') break;
	}

	*number=0;
	for (; i<strlen(command) && '0'<=command[i] && command[i]<='9'; i++)
	{
		*number=*number*10+(int) command[i]-'0';
	}
}

/*======================================================================*
                            dealWithCommand
 *======================================================================*/

void dealWithCommand(char* command)
{
	while(1)
	{
		strlwr(command);
		if (strcmp(command,"clear")==0)
		{
			clearScreen();
			sys_clear(tty_table);
			return ;
		}
		if (strcmp(command,"help")==0)
		{
			help();
			return ;
		}
		if (strcmp(command,"show")==0)
		{
			show();
			return ;
		}
		if (strcmp(command,"shutdown")==0)
		{
			shutdown();
			while(1);
		}
		char str[10] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
		int number;
		readOneStringAndOneNumber(command,str,& number);
		if (strcmp(str,"kill")==0)
		{
			if (number<0 || number>NR_TASKS+NR_PROCS)
			{
				printf("No found this process!!");
			}
			else if (number==0 || number==8)
			{
				printf("You do not have sufficient privileges\n");
			}
			else if (2<=number && number <=7)
			{
				proc_table[number].state=kREADY;
				printf("kill process %d successful\n",number);
			}
			return ;
		}
		if (strcmp(str,"start")==0)
		{
			if (number<0 || number>NR_TASKS+NR_PROCS)
			{
				printf("No found this process!!");
			}
			else if (number==0 || number==8)
			{
				printf("You do not have sufficient privileges\n");
			}
			else if (2<=number && number <=7)
			{
				proc_table[number].state=kRUNNING;
				printf("start process %d successful\n",number);
			}
			return ;
		}
		printf("can not find this command\n");
	}
	
}

/*======================================================================*
                               Terminal
 *======================================================================*/
void Terminal()
{
	TTY *p_tty=tty_table;
	p_tty->startScanf=0;
	while(1)
	{
		printf("DB=>");
		//printf("<Ticks:%x>", get_ticks());
		openStartScanf(p_tty);
		while (p_tty->startScanf) ;
		dealWithCommand(p_tty->str);
	}
}


/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	int i = 0;
	while(1){
		printf("B");
		milli_delay(1000);
	}
}

/*======================================================================*
                               TestC
 *======================================================================*/
void TestC()
{
	int i = 0;
	while(1){
		printf("C");
		milli_delay(1000);
	}
}

/*======================================================================*
                               TestD
 *======================================================================*/
void TestD()
{
	int i=0;
	while (1)
	{
		printf("D");
		milli_delay(1000);
	}
}

/*======================================================================*
                               TestE
 *======================================================================*/
void TestE()
{
	int i=0;
	while (1)
	{
		printf("E");
		milli_delay(1000);
	}
}



/*======================================================================*
								calculator
*=======================================================================*/

TTY *calculatorTty=tty_table+2;

#define length  100

char token;
int pos;
char str[length];

double exp();
double term();
double factor();


void match(char expectedToken);
void error();

int isdigit(char ch)
{
	if(ch>='0'&&ch<='9')
		return 1;
	return 0;
}

char getChar(char* str)
{
	if(pos>=length) return '\0';
	return str[pos++];
}

void getString(char* str)
{	
	int i;
	char temp;
	for (i=0; i<calculatorTty->len && calculatorTty->str[i]==' '; i++);
	while(i<length && i<calculatorTty->len)
	{
		temp = calculatorTty->str[i];
		if(temp == '\n') break;
		if(temp != ' ' && temp != '\t')
			str[i++] = temp;
		else continue;
	}
	str[i] = '\0';
}



void error()
{
	printf("Error!\n");
}

double readNum(char bgn)
{
	double num = bgn - '0';
 	int deci_num = 0;
	bgn = getChar(str);
	if(!isdigit(bgn) && bgn != '.')
	{
		pos--;
		return num;
	}
	while(isdigit(bgn) || bgn == '.')
	{
		if(bgn == '.')
			deci_num ++;
		else
			num = num*10 + (double)(bgn -'0');
		bgn = getChar(str);
	}
	while(deci_num)
	{
		num /=10;
		deci_num--;
	}
	pos--;
	return num;
}


void match(char expectedToken)
{
	if (expectedToken == token)
		token = getChar(str);
	else
		error();
}

double exp()
{
	double temp = term();
	while (token == '+' || token == '-')
		switch(token)
		{
			case '+': match('+');
					  temp += term();
					  break;
			case '-': match('-');
					  temp -= term();
					  break;
		}
	return temp;
}


double factor()
{
	double temp;
	if (token == '(')
	{
		match('(');
		temp = exp();
		match(')');
	} else if (isdigit(token))
	{
		temp = readNum(token);
		token = getChar(str);
	} else
		error();
	return temp;
}


double term()
{
	double temp = factor();
	while (token == '*' || token == '/')
		switch(token)
		{
			case '*': match('*');
					  temp *= factor();
					  break;
			case '/': match('/');
					  temp /= factor();
					  break;
		}
	return temp;
}

void printNum(double d)
{
	int m = d;
	int digit = m;
	d-=m;
	int precision = 3;
	char num[precision+1];
	num[precision] = '\0';
	int i = 0;
	while(precision--)
	{
		d*=10;
		m = d;
		num[i++] = m+'0';
		d-=m;
	}
	int flag = 0;
	if(d*10>5) 
		flag = 1;
	while(--i&&flag)
	{
		if(num[i]=='9')
			num[i] = '0';
		else {
			num[i]++;
			flag = 0;
		}
	}
	printf("%d.%s\n",digit,num);
}

void calculator()
{
	printf("Hello! This is a calculator application!\n");
	while(1){
		printf("please enter an equation:  \n");
		openStartScanf(calculatorTty);
		while (calculatorTty->startScanf) ;
		getString(str);
		pos = 0;
		token = getChar(str);
		if (token == 'q')
			break;
		double result = exp();
		
		if (token == '\0')
		{
			printNum(result);
		}
		else
			error();
	}
	printf("Bye! \n");
}



/*======================================================================*
						Game
*=======================================================================*/


TTY *GameTty=tty_table+3;

void readNumber(int* x)
{
	openStartScanf(GameTty);
	while (GameTty->startScanf);
	int i=0;
	*x=0;
	for (i=0; i<GameTty->len && GameTty->str[i]==' '; i++);
	for (; i<GameTty->len && GameTty->str[i]!=' ' && GameTty->str[i]!='\n'; i++)
	{
		*x=(*x)*10+(int) GameTty->str[i]-48;
	}
}


void Game()
{	
	while (1)
	{
		printf("There is a number between 1 and 1000 , guess what is it according to the tips.\n");
		int iTarget,iNumber;
		
		iTarget = (int)(get_ticks() * 2.7182818) % 1000 + 1;//利用系统时钟函数产生随机数	
		do
		{
			printf("enter your number:");			
			readNumber(&iNumber);
			if(iNumber<iTarget)
				printf("What you guess is smaller than the target.\n");
			else if(iNumber>iTarget)
				printf("What you guess is bigger than the target.\n");
			else{
				printf("Congratulations.You are right.\n");
				printf("10 Seconds later another game start.\n");
				clearScreen();
				milli_delay(10000);
			}
		}while(iNumber != iTarget);
	}
}


/*======================================================================*
								Calendar
*=======================================================================*/

TTY *calendarTty=tty_table+4;
#define N 7
			
void readTwoNumberInCalendar(int* x,int* y)
{
	int i=0;
	*x=0;
	*y=0;
	for (i=0; i<calendarTty->len && calendarTty->str[i]==' '; i++);
	for (; i<calendarTty->len && calendarTty->str[i]!=' ' && calendarTty->str[i]!='\n'; i++)
	{
		*x=(*x)*10+(int) calendarTty->str[i]-48;
	}
	for (i; i<calendarTty->len && calendarTty->str[i]==' '; i++);
	for (; i<calendarTty->len && calendarTty->str[i]!=' ' && calendarTty->str[i]!='\n'; i++)
	{
		*y=(*y)*10+(int) calendarTty->str[i]-48;
	}
}
			
void calendar()
{
	printf("Hello! This is a calendar application, you can use it as follows!\n");	
	int year, month, x, y;
	while (1)
	{
		printf("Please input the year and month: ");
		openStartScanf(calendarTty);
		while (calendarTty->startScanf) ;
		readTwoNumberInCalendar(&x,&y);
		year=x;
		month=y;
		rili(year,month);		
	}
	printf("byebye\n");
}
void print(int day,int tian)
{
	int a[N][N],i,j,sum=1;
	for(i=0,j=0;j<7;j++)
	{
		if(j<day)
			printf("    ");
		else
		{
			a[i][j]=sum;
			printf("   %d",sum++);
			// printf("aaa\n");
		}
	}
	printf("\n");
	for(i=1;sum<=tian;i++)
	{
		for(j=0;sum<=tian&&j<7;j++)
		{
			a[i][j]=sum;
			if (sum<10)
			{
				printf("   %d", sum++);
			}
			else{
				printf("  %d",sum++);
			}
		}
		printf("\n");
	}
}

int duo(int year)
{
	if(year%4==0&&year%100!=0||year%400==0)
		return 1;
	else
		return 0;
}


int rili(int year,int month)
{
	int day,tian,preday,strday;
	char monthStr[10];
	switch(month)
	{
		case 1:
		tian=31;
		preday=0;
		strcpy(monthStr,"January");
		break;
		case 2:
		tian=28;
		preday=31;
		strcpy(monthStr,"Febrary");
		break;
		case 3:
		tian=31;
		preday=59;
		strcpy(monthStr,"March");
		break;
		case 4:
		tian=30;
		preday=90;
		strcpy(monthStr,"April");
		break;
		case 5:
		tian=31;
		preday=120;
		strcpy(monthStr,"May");
		break;
		case 6:
		tian=30;
		preday=151;
		strcpy(monthStr,"June");
		break;
		case 7:
		tian=31;
		preday=181;
		strcpy(monthStr,"July");
		break;
		case 8:
		tian=31;
		preday=212;
		strcpy(monthStr,"August");
		break;
		case 9:
		tian=30;
		preday=243;
		strcpy(monthStr,"September");
		break;
		case 10:
		tian=31;
		preday=273;
		strcpy(monthStr,"October");
		break;
		case 11:
		tian=30;
		preday=304;
		strcpy(monthStr,"November");
		break;
		default:
		tian=31;
		preday=334;
		strcpy(monthStr,"December");
	}
	if(duo(year)&&month>2)
	preday++;
	if(duo(year)&&month==2)
	tian=29;
	day=((year-1)*365+(year-1)/4-(year-1)/100+(year-1)/400+preday+1)%7;
	printf("************%s.%d***********\n",monthStr,year);
	printf(" SUN MON TUE WED THU FRI SAT\n");
	print(day,tian);
}


/*======================================================================*
								Task Manager
*=======================================================================*/
TTY *taskMngTty=tty_table+5;

void dealMngCmd(char* command)
{
	while(1)
	{
		strlwr(command);
		char str[10] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
		int number;
		readOneStringAndOneNumber(command,str,& number);
		if (strcmp(str,"kill")==0)
		{
			if (number<0 || number>NR_TASKS+NR_PROCS)
			{
				printf("No found this process!!");
			}
			else if (number==0 || number==1)
			{
				printf("You do not have sufficient power\n");
			}
			else if (2<=number && number <=7)
			{
				proc_table[number].state=kREADY;
				printf("kill process %d successful\n",number);
			}else
			printf("no such process!\n");
			return ;
		}
		if (strcmp(str,"start")==0)
		{
			if (number<0 || number>NR_TASKS+NR_PROCS)
			{
				printf("No found this process!!");
			}
			else if (number==0 || number==8)
			{
				printf("You do not have sufficient privileges\n");
			}
			else if (2<=number && number <=7)
			{
				proc_table[number].state=kRUNNING;
				printf("start process %d successful\n",number);
			}
			return ;
		}
		printf("can not find this command\n");
	}
	
}


void taskManager()
{	
	printf("Hello! You can manager process as follows!\n");
	printf("Enter \"kill + number\" to kill process\n");	
	printf("Enter \"start + number\" to start process\n");	
	
	while(1)
	{
		openStartScanf(taskMngTty);
		while (taskMngTty->startScanf) ;
		dealMngCmd(taskMngTty->str);
	}
}

/*======================================================================*
				animation
*=======================================================================*/

void DisPlayAnimation()//displayed when OS starts
{
	int color = 0x7f;

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////	
	clearScreen();
	disp_color_str("                                                \n",0x33);	
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	milli_delay(1);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////	
	clearScreen();
	disp_str("                               \n");
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	milli_delay(1);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////	
	clearScreen();
	disp_str("                                                \n");
	disp_str("                                                \n");
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	milli_delay(1);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////	
	clearScreen();
	disp_str("                                                \n");
	disp_str("                                                \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	milli_delay(1);
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////	
	clearScreen();
	disp_str("                                                \n");
	disp_str("                                                \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	milli_delay(1);
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////	
	clearScreen();
	disp_str("                                                \n");
	disp_str("                                                \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	milli_delay(1);
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////	
	clearScreen();
	disp_str("                                                \n");
	disp_str("                                                \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	milli_delay(1);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////	
	clearScreen();
	disp_str("                                                \n");
	disp_str("                                                \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	milli_delay(1);
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////	
	clearScreen();
	disp_str("                                                \n");
	disp_str("                                                \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	milli_delay(1);
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////	
	clearScreen();
	disp_str("                                                \n");
	disp_str("                                                \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str("         ",0xFF);
	disp_str("                              \n");
	disp_color_str("                                                \n",0x33);
	disp_color_str("                                                \n",0x33);
	milli_delay(1);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////	
	clearScreen();
	disp_str("                                                \n");
	disp_str("                                                \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str("         ",0xFF);
	disp_str("                                    \n");
	disp_str("                                                \n");
	disp_color_str("                                                \n",0x33);
	milli_delay(1);
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////	
	clearScreen();
	disp_str("                                                \n");
	disp_str("                                                \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str(" ",0xFF);
	disp_str("                              \n");
	disp_str("                  ");
	disp_color_str("         ",0xFF);
	disp_str("                                    \n");
	disp_str("                                                \n");
	disp_str("                                                \n");
	milli_delay(1);


	milli_delay(5);	
	clearScreen();

}

/*======================================================================*
							display welcome
*=======================================================================*/

void displayWelcome()
{
	clearScreen();
	
	disp_str("=============================================================================\n");
	disp_str("========                          Welcom To ");
	disp_color_str("LINX",0xB);
	disp_str("                   ==========\n");
	disp_str("========                            Created By                       ========\n");
	disp_str("========                            Lidong Liu                       ========\n");
	disp_str("=============================================================================\n");
	disp_str("    ===============================================================\n");
	disp_str("       =====  help         --------  show the help menu     =====\n");
	disp_str("       =====  clear        --------  clear screen           =====\n");	
	disp_str("       =====  show         --------  show the process info  =====\n");
	disp_str("       =====  shutdown     --------  shut down the computer =====\n");
	disp_str("       =====  F1           --------  return main menu       =====\n");
	disp_str("       =====  F2           --------  show the process run   =====\n");
	disp_str("       =====  F3           --------  calculator             =====\n");
	disp_str("       =====  F4           --------  Game                   =====\n");
	disp_str("       =====  F5           --------  calendar               =====\n");
	disp_str("       =====  F6           --------  task manager           =====\n");
	disp_str("    ===============================================================\n");
	disp_str("\n");
}

/*======================================================================*
									shutdown
*=======================================================================*/

void shutdown()
{
	clearScreen();
	disp_str("\n\n\n\n\n");
	disp_color_str("                             b       Y   Y     eeee      \n",0x1); 	
	disp_color_str("                             b        y  y    e    e     \n",0x1); 	
	disp_color_str("                             b b       y y    eeeeee     \n",0x2); 	
	disp_color_str("                             b   b       y    e          \n",0x2); 	
	disp_color_str("                             b   b       y    e          \n",0x3); 	
	disp_color_str("                             b b      yyy      eeee      \n",0x3); 
	
	disp_str("\n\n\n"); 	
	disp_color_str("                     ---------------- Made BY -------------\n\n",0xB); 	
	disp_color_str("                     --              Lidong Liu          --\n\n",0xF); 	
	disp_color_str("                     -----------      Goodbye     ---------\n\n",0xD);
}

