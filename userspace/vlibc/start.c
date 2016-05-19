int main(int argc, char **argv, char **envp);
void exit(int status);

void _start(int argc, char **argv, char **envp) {

	int result;

	result=main(argc,argv,envp);

	exit(result);

}
