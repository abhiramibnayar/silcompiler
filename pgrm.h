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
strconst=10;
array[i]=11;
returnstmt=12;
functioncallstmt=13;
arglistnode=14;
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
  struct localsymboltable *lsymtab;
  struct globalsymboltable *gsymtab;
}tnode;

typedef struct ntype
{
  int vartype;
  int arraysize;
  int width;
  struct ntype *elemtype;
}ntype;

typedef struct globalsymboltable
{
  char *name;
  struct ntype *type;
  int size;
  int binding;
  struct parameterlist *plist;
  int functionlabel;
  struct globalsymboltable *nextentry;
}globalsymboltable;


typedef struct parameterlist
{
  char *name;
  struct ntype *type;
  struct parameterlist *nextentry;
}parameterlist;

typedef struct localsymboltable
{
  char *name;
  struct ntype *type;
  int size;
  int binding;
  struct localsymboltable *nextentry;
}localsymboltable;

struct tnode* makenode(int val, int type,char *op,int nodetype, struct tnode *left, struct tnode *right,struct tnode *centre);
int getReg(void);
void freeReg(void);
int codegen(struct tnode*);
int getLabel(void);
void pushws(int,int);
void popws();

void gstinstall(char *name, struct ntype *type, int size, struct parameterlist *plist);
struct globalsymboltable* gstlookup(char *name);
void printglobalsymboltable();

void lstinstall(char*,struct ntype*);
struct localsymboltable* lstlookup(char*);
void printlocalsymboltable();

struct globalsymboltable *gsthead;
struct localsymboltable *lsthead;

FILE *fp;
int regallocate=3;
int a[26];
int label=1;
int whilestack[40];
int wstop=0;
int staticmem=4096;
int decls=1;
int stores=0;
int localbinding=0;
int mainfunction=0;
