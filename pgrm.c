
struct tnode* makenode(int val, int type,char *op,int nodetype, struct tnode *left, struct tnode *right,struct tnode *centre)
{
  struct tnode *temp=(struct tnode*) malloc(sizeof(struct tnode));
  temp->val=val;
  temp->type=type;

  if(op!=NULL)
  {
    temp->op=(char*)malloc(sizeof(char*));
    strcpy(temp->op,op);
  }
  else
  temp->op=op;
  temp->nodetype=nodetype;
  temp->left=left;
  temp->right=right;
  temp->centre=centre;
  temp->symtab=NULL;
  if(temp->nodetype==2 && decls==0)
  {
    temp->symtab=lookup(temp->op);

    if(temp->symtab==NULL)
    {
      yyerror("variable not declared");
      exit(1);
    }
    temp->type=temp->symtab->type;

  }
  if((temp->op!=NULL && (strcmp(temp->op,"=")==0))||(type!=0 && left!=NULL && right!=NULL))//if the node isnt typeless
  {
    if(left->type!=right->type)
     {
    printf("Type Mismatch\nCompilation Interrupted");
    exit(1);
     }
  }


  return temp;
}


void printtree(struct tnode *t)
{
  switch (t->nodetype) {
    case 1:
    printf("%d ", t->val);
    break;

    case 2:
    printf("%s ", t->op);
    printf(" symboltable\n");
    printf("%s %d %d\n",t->symtab->name,t->symtab->size,t->symtab->binding);
    break;

    case 3:
    printf("\nread\n");
    printtree(t->left);
    break;

    case 4:
    printf("\nwrite\n");
    printtree(t->left);
    break;

    case 5:
    printf("\nconnector \n left \n");
    printtree(t->left);
    printf("\nright\n");
    printtree(t->right);
    break;

    case 6:
    case 8:
    printf("\n%s\n",t->op);
    printf("left\n");
    printtree(t->left);
    printf("\nright\n");
    printtree(t->right);
    return;
    break;

    case 7:
    printf("\n%s\n",t->op);
    printf("left\n");
    printtree(t->left);
    printf(" ");
    if(t->centre!=NULL)
    {
      printf("\ncentre\n");
      printtree(t->centre);
    }
    printf("\nright\n ");
    printtree(t->right);
    return;
    break;

    case 9:
    printf("\n%s\n",t->op);
    return ;
    break;

    case 10:
    printf("\n%s\n",t->op);
    return ;
    break;

    default:
    break;

}
return ;
}

int getLabel(){
  return label++;
}




int getReg()
{
  if(regallocate<20)
  return regallocate++;
  else
  printf("Can't assign register\n");
  return -1;
}

void freeReg(){
  if(regallocate>3)
  regallocate--;
  return ;
}


int codegen(struct tnode *t)
{
  int p,l,r,a,label1,label2;
  char opr;
  switch (t->nodetype) {
    case 1: //return const in a register
    p=getReg();
    fprintf(fp, "MOV R%d, %d\n",p,t->val );
    return p;
    break;

    case 2:
    p=getReg(); //return variable in register
    fprintf(fp, "MOV R%d, [%d]\n",p,((t->symtab->binding)+(t->val-1)));
    return p;
    break;

    case 3: //code for read
    fprintf(fp, "MOV R0,\"Read\"\nPUSH R0\nMOV R0,-1\nPUSH R0\n");
    a=t->left->symtab->binding+(t->left->val-1);
    fprintf(fp, "MOV R0,%d\n",a);
    fprintf(fp, "PUSH R0\nPUSH R0\nPUSH R0\nCALL 0\nPOP R0\nPOP R1\nPOP R1\nPOP R1\nPOP R1\n");
    return 1;
    break;

    case 4://code for write
    fprintf(fp, "MOV R2, \"Write\"\nPUSH R2\n");
    p=codegen(t->left);
    fprintf(fp, "MOV R2,-2\nPUSH R2\nPUSH R%d\nPUSH R0\nPUSH R0\nCALL 0\nPOP R0\nPOP R1\n",p);
    fprintf(fp, "POP R1\nPOP R1\nPOP R1\n");
    freeReg();
    return 1;
    break;

    case 5://code for connectors
    l=codegen(t->left);
    r=codegen(t->right);

    return 1;
    break;

    case 6://code for arithmetic operators
    opr=*(t->op);
    switch (opr) {
      case '+':
      l=codegen(t->left);
      r=codegen(t->right);
      fprintf(fp, "ADD R%d, R%d\n",l,r);
      freeReg();
      return l;
      break;

      case'-':
      l=codegen(t->left);
      r=codegen(t->right);
      fprintf(fp, "SUB R%d, R%d\n",l,r);
      freeReg();
      return l;
      break;

      case '*':
      l=codegen(t->left);
      r=codegen(t->right);
      fprintf(fp, "MUL R%d, R%d\n",l,r);
      freeReg();
      return l;
      break;

      case '/':
      l=codegen(t->left);
      r=codegen(t->right);
      fprintf(fp, "DIV R%d, R%d\n",l,r);
      freeReg();
      return l;
      break;

      case '=':
      l=codegen(t->right);
      a=t->left->symtab->binding+(t->left->val)-1;
      fprintf(fp,"MOV [%d],R%d\n",a,l);
      freeReg();
      return 1;
      break;

      default:
      break;
    }

    case 7: //while and if
    if(strcmp("while",t->op)==0)
    {
      label1=getLabel();
      label2=getLabel();
      pushws(label1,label2);
      fprintf(fp, "L%d:\n",label1);
      l=codegen(t->left);

      fprintf(fp, "JZ R%d,L%d\n",l,label2);
      freeReg();
      r=codegen(t->right);

      fprintf(fp, "JMP L%d\n",label1);
      fprintf(fp, "L%d:\n",label2);
      popws();
      return 1;
    }
    else if(strcmp("if",t->op)==0)
    {
      label1=getLabel();
      label2=getLabel();
      if(t->centre!=NULL)
      {


      l=codegen(t->left);
      fprintf(fp, "JZ R%d,L%d\n",l,label1);
      freeReg();

      l=codegen(t->centre);

      fprintf(fp, "JMP L%d\n",label2 );
      fprintf(fp, "L%d:\n",label1 );
      r=codegen(t->right);

      fprintf(fp, "L%d:\n",label2);
    }
    else{
      l=codegen(t->left);
      fprintf(fp, "JZ R%d,L%d\n",l,label1);
      freeReg();

      r=codegen(t->right);
      fprintf(fp, "JMP L%d\n",label2 );
      fprintf(fp, "L%d:\n",label1 );
      fprintf(fp, "L%d:\n",label2);
    }
      return 1;
    }
    else{
      fprintf(fp, "weird function with if-while\n");
    }
    break;
    case 8: //relationaloperators
    if(strcmp("<",t->op)==0)
    {
        l=codegen(t->left);
        r=codegen(t->right);
        fprintf(fp, "LT R%d,R%d\n",l,r);
        freeReg();
        return l;

    }
    else if(strcmp("<=",t->op)==0)
    {
      l=codegen(t->left);
      r=codegen(t->right);
      fprintf(fp, "LE R%d,R%d\n",l,r);
      freeReg();
      return l;
    }
    else if(strcmp(">",t->op)==0)
    {
      l=codegen(t->left);
      r=codegen(t->right);
      fprintf(fp, "GT R%d,R%d\n",l,r);
      freeReg();
      return l;
    }
    else if(strcmp(">=",t->op)==0)
    {
      l=codegen(t->left);
      r=codegen(t->right);
      fprintf(fp, "GE R%d,R%d\n",l,r);
      freeReg();
      return l;
    }
    else if(strcmp("==",t->op)==0)
    {
      l=codegen(t->left);
      r=codegen(t->right);
      fprintf(fp, "EQ R%d,R%d\n",l,r);
      freeReg();
      return l;
    }
    else if(strcmp("!=",t->op)==0)
    {
      l=codegen(t->left);
      r=codegen(t->right);
      fprintf(fp, "NE R%d,R%d\n",l,r);
      freeReg();
      return l;
    }
    else
    {
      fprintf(fp, "weird relationaloperators\n" );
    }
    break;

    case 9: //break and continue
    if(strcmp("break",t->op)==0)
    {
      if(wstop!=0){
      fprintf(fp, "JMP L%d\n",whilestack[wstop-1]);
      popws();
    }
    }
    if(strcmp("continue",t->op)==0)
    {
      if(wstop!=0)
      fprintf(fp, "JMP L%d\n",whilestack[wstop-2]);
    }
    return 1;
    break;

    case 10: //string constants
    p=getReg();
    fprintf(fp, "MOV R%d, %s\n",p,t->op );
    return p;
    break;
    default:
    break;


  }


}

void pushws(int a,int b)
{
  whilestack[wstop++]=a;
  whilestack[wstop++]=b;

}
void popws(){
  wstop--;
  wstop--;
}

struct symboltable* lookup(char *name)
{
  struct symboltable *sttemp=sthead;
  if(sthead==NULL)
  return NULL;

  while(strcmp(sttemp->name,name)!=0)
  {
    sttemp=sttemp->next;
    if(sttemp==NULL)
    return NULL;
  }
  return sttemp;
}

void install(char *name, int type, int size)
{
  struct symboltable *sttemp=(struct symboltable*)malloc(sizeof(struct symboltable));
  sttemp->name=(char*)malloc(sizeof(char*));
  sttemp->name=name;
  sttemp->type=type;
  sttemp->size=size;
  sttemp->next=NULL;
  sttemp->binding=staticmem;
  staticmem=staticmem+size;
  if(sthead==NULL)
  {
    sthead=sttemp;
    return ;
  }
  struct symboltable *trv=sthead;
  while(trv->next!=NULL)
  {
    trv=trv->next;
  }
  trv->next=sttemp;
  return ;
}

void printsymtable()
{
  struct symboltable *trv=sthead;
  printf("Symboltable\n");
  while(trv!=NULL)
  {
    printf("%s, %d, %d, %d\n",trv->name,trv->type,trv->size,trv->binding );
    trv=trv->next;
  }
}
