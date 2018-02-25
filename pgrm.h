/*
nodetype
numb=1;
variable=2;
read=3,
write=4,
connector=5,
arithmeticoperators=6;
if-while=7;
relationaloperators=8;
breakcont=9;
strconst=10
*/


/*
type
integer=1;
boolean=2;
string=3;
typeless=0;
*/
//max 20 nested while loops
typedef struct tnode
{
  int val; //value of a NUM token
  int type; // type of variable
  char *op; // variable name/operatorsname
  int nodetype; //what kind of non-leaf node.
  struct tnode *left;
  struct tnode *right;
  struct tnode *centre;
  struct symboltable *symtab;
}tnode;

typedef struct symboltable
{
  char *name;
  int type;
  int size;
  int binding;
  struct symboltable *next;
}symboltable;
struct tnode* makenode(int val, int type,char *op,int nodetype, struct tnode *left, struct tnode *right,struct tnode *centre);
//int evaluate(struct tnode*);
void printtree(struct tnode*);
int getReg(void);
void freeReg(void);
int codegen(struct tnode*);
int getLabel(void);
void pushws(int,int);
void popws();

struct symboltable* lookup(char*);
void install(char*, int, int);
void printsymtable();

struct symboltable *sthead;

FILE *fp;
int regallocate=3;
int a[26];
int label=0;
int whilestack[40];
int wstop=0;
int staticmem=4096;
int decls=1;
