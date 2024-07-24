extern void myprint();
extern void putchar(unsigned char c);
extern void print_string(const char*);
void print(const char* c);
void DISPLAY()
{
	myprint();
	char str[]= "hello\n";
	print(str);
//	print("\n");	//不知道为什么这样不行
	char str2[] = "ni haoawaawdsr";//14个+一个'\0'
//	print("ni hao");
	print(str2);
	print(str);

	//不知道为什么打印不出来字符串
	print_string(str);

	//循环，以暂停
	while(1){
	}
}
int getlen(const char* c)
{
	int count=0;
	while(c[count] != '\0')
	{
		count++;
	}
	return count;
}

void print(const char* c)
{
	for(int i = 0;i < getlen(c); i++)
	{
		putchar(c[i]);
	}
}



