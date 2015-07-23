
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
		p->ticks=p->priority;
		p->whichQueue=2;
	}
	else 
	{
		thirdQueue[thirdLen]=p;
		thirdLen++;
		p->ticks=5;
		p->whichQueue=3;
	}
}

/*======================================================================*
                            tinix_main
 *======================================================================*/
PUBLIC int tinix_main()
{
	//disp_str("-----\"tinix_main\" begins-----\n");
	clearScreen();
	disp_str("         *************************************************************\n");
	disp_str("        *                                                             \n");
	disp_str("       * **********    ******    ***      **    ******    **    **    \n");
	disp_str("      *      **          **      ****     **      **        *  *      \n");
	disp_str("     *       **          **      ** **    **      **         **       \n");
	disp_str("    *        **          **      **  **   **      **         **       \n");
	disp_str("   *         **          **      **   **  **      **         **       \n");
	disp_str("  *          **          **      **    ** **      **         **       \n");
	disp_str(" *           **          **      **     ****      **       *    *     \n");
	disp_str("*            **        ******    **      ***    ******   **      **   \n");
	disp_str("*                                                                     \n");
	disp_str("**********************************************************************\n");
	//disp_color_str("Hello World1\n", 0x1);//blue
	// disp_color_str("Hello World2\n", 0x2);//green
	// disp_color_str("Hello World3\n", 0x3);//blueness
	// disp_color_str("Hello World4\n", 0x4);//red
	// disp_color_str("Hello World5\n", 0x5);//pink
	// disp_color_str("Hello World6\n", 0x6);//brown
	// disp_color_str("Hello World7\n", 0x7);//gray
	// disp_color_str("Hello World8\n", 0x8);//dark gray
	// disp_color_str("Hello World9\n", 0x9);//light blue
	// disp_color_str("Hello WorldA\n", 0xA);//light green
	// disp_color_str("Hello WorldB\n", 0xB);//light blueness
	// disp_color_str("Hello WorldC\n", 0xC);//light red
	// disp_color_str("Hello WorldD\n", 0xD);//light pink
	// disp_color_str("Hello WorldE\n", 0xE);//light yellow
	// disp_color_str("Hello WorldF\n", 0xF);//white
	// disp_color_str("Hello WorldAE\n", 0xAE);//back:green font:yellow flicker
	// disp_color_str("Hello WorldAF\n", 0xAF);//back:green font:white flicker
	// disp_color_str("Hello WorldBE\n", 0xBE);//back:blueness font:yellow flicker
	// disp_color_str("Hello WorldBF\n", 0xBF);//back:blueness font:white flicker
	
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
	proc_table[3].priority =  3;
	proc_table[4].priority =  7;
	proc_table[5].priority =  10;
	proc_table[6].priority =  10;//calendar
	proc_table[7].priority =  20;//guess


	firstLen=firstHead=secondLen=thirdLen=0;
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
	proc_table[6].nr_tty = 2; //calendar Alt+F3
	proc_table[7].nr_tty = 3;//guss game Alt+F4

	k_reenter	= 0;
	ticks		= 0;

	p_proc_ready	= proc_table;

	init_clock();

	restart();

	while(1){}
}

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


void help()
{
	printf("           *////////////////////////////////////////////*/\n");
	printf("                   design by Doubi && LiangPuHe         \n");
	printf("           *////////////////////////////////////////////*/\n");
	printf("\n");
	printf("      *////////////////////////////////////////////////////////*\n");
	printf("      *////  help         --------  shwo the help menu     ////*\n");
	printf("      *////  clear        --------  clear screen           ////*\n");
	printf("      *////  alt+F2       --------  show the process run   ////*\n");
	printf("      *////  alt+F3       --------  calendar               ////*\n");
	printf("      *////  kill 2~5     --------  kill the process 2~5   ////*\n");
	printf("      *////  start 2~5    --------  start the process 2~5  ////*\n");
	printf("      *////  show         --------  show the process state ////*\n");
	printf("      *////////////////////////////////////////////////////////*\n");
	printf("\n");
}

void show()
{
	PROCESS* p;
	int i;
	for (i=0; i<NR_TASKS+NR_PROCS;i++)
	{
		p=&proc_table[i];
		printf("process%d:",p->pid);
		switch (p->state)
		{
		case kRUNNABLE:
			printf("    Runnable\n");
			break;
		case kRUNNING:
			printf("    Running\n");
			break;
		case kREADY:
			printf("    Ready\n");
			break;
		}
		printf(p->whichQueue);
		printf("\n");
	}
}

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

void dealWithCommand(char* command)
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
	char str[100];
	int number;
	readOneStringAndOneNumber(command,str,& number);
	if (strcmp(str,"kill")==0)
	{
		if (number<0 || number>NR_TASKS+NR_PROCS)
		{
			printf("No found this process!!");
		}
		else if (number==0 || number==6)
		{
			printf("You do not have sufficient privileges\n");
		}
		else if (2<=number && number <=5)
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
		else if (number==0 || number==6)
		{
			printf("You do not have sufficient privileges\n");
		}
		else if (2<=number && number <=5)
		{
			proc_table[number].state=kRUNNABLE;
			printf("start process %d successful\n",number);
		}
		return ;
	}
	printf("can not find this command\n");
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

void TestD()
{
	int i=0;
	while (1)
	{
		printf("D");
		milli_delay(1000);
	}
}

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
				Game2048
*=======================================================================*/

//TTY *goBangGameTty=tty_table+2;


// void displayGameState()
// {

// 	sys_clear(goBangGameTty);
// 	disp_str("start");

// }



/*======================================================================*
				guess game
*=======================================================================*/
TTY *gussTTy = tty_table+1;
void guess()
{
	char gamer;  // 玩家出拳
    int computer;  // 电脑出拳
    int result;  // 比赛结果

    // 为了避免玩一次游戏就退出程序，可以将代码放在循环中
    while (1){
        printf("This is a guess game, please input your choice: ");
        printf("A:scissors\nB:stone\nC:cloth\nD:end game\n");
        do 
        {
        	gamer = getUserInput();
        }
        while(gamer == -1);
        if (gamer == 0)
        {
        	return;
        }
       
        computer=getRandom(5);  // 产生随机数并取余，得到电脑出拳
        result=(int)gamer+computer;  // gamer 为 char 类型，数学运算时要强制转换类型
        printf("Computer....:");
        switch (computer)
        {
            case 0:printf("scissors\n");break; //4    1
            case 1:printf("stone\n");break; //7  2
            case 2:printf("cloth\n");break;   //10 3
        }
        printf("Your....:");
        switch (gamer)
        {
            case 4:printf("scissors\n");break;
            case 7:printf("stone\n");break;
            case 10:printf("cloth\n");break;
        }
        if (result==6||result==7||result==11) printf("You win!");
        else if (result==5||result==9||result==10) printf("Computer win!");
        else printf("deuce");

        milli_delay(1000);
        clearScreen();  // 暂停并清屏

    }
    //while(1);
}

int getRandom(int i) {
	return i%3;
}

int getUserInput() {
	openStartScanf(gussTTy);
	while(gussTTy->startScanf) ;
	if (strcmp(gussTTy->str, "a") == 0)
	{
		return 4;
	}
	else if (strcmp(gussTTy->str, "b") == 0) {
		return 7;
	}
	else if (strcmp(gussTTy->str, "c") == 0) {
		return 10;
	}
	else if (strcmp(gussTTy->str, "d") == 0) {
		return 0;
	}
	else {
		printf("您的输入有误\n");
		return -1;
	}
}

/*======================================================================*
				Calender of July, 2015
*=======================================================================*/

void calendar()
{
		int a,b,c,d,e,f,i,j,k,n,m,year;
		b=2015;
		c=year%4;
		d=year/4;
		e=d*1461+c*365;
		f=e%7;
		j=f;         
		m=j;
		printf("*****The calendar of 2015.7*****\n");
		for(a=1;a<=12;a++)   
   		{
			if(a==7)
			{
    				if(a==1||a==3||a==5||a==7||a==8||a==10||a==12)k=31;
    				else if(a==4||a==6||a==9||a==11)k=30;
         			else if((year%4==0&&year%100!=0)||(year%400==0))k=29;           
              			else k=28;
    				printf("                      %dJuly                    \n",a);
    				printf("   STAT    SUN    MON    TUE    WED   THUR    FRI\n");
   				m=j;
    				if(m<=5)m=m+1;
    				else m=m-6;

    				for(n=1;n<=m;n++)printf("       ");

    				for(i=1;i<=k;i++,j++)
       				{
					if(j==7)j=0;
        				if(i<10)printf("      %d",i);
        				else printf("     %d",i);
        				if(j==5)printf("\n");
       				}
    				printf("\n\n\n");
			}

   		}
	while(1);
}


