
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

  if(nodetype==2||nodetype==13)
  {
    temp->lsymtab=lstlookup(temp->op);
    temp->gsymtab=gstlookup(temp->op);
    if(temp->lsymtab!=NULL)
    {
    temp->type=temp->lsymtab->type->vartype;
    }
    else if(temp->gsymtab!=NULL)
    {
    temp->type=temp->gsymtab->type->vartype;
    }
  }

  if(temp->nodetype==2 && decls==0)
  {

    if(temp->lsymtab==NULL && temp->gsymtab==NULL)
    {
      printf("%s %d\n",temp->op,decls);
      yyerror("variable not declared");
      exit(1);
    }

  }
  if((temp->op!=NULL && (strcmp(temp->op,"=")==0))||(type!=0 && left!=NULL && right!=NULL))//if the node isnt typeless
  {

    if(left->type!=right->type)
     {
    printf("Type Mismatch\nCompilation Interrupted");
    printf("The types are %s %d %s %d \n",left->op,left->type,right->op, right->type );
    exit(1);
     }
  }


  return temp;
}



int getLabel(){
  return label++;
}

int getReg()
{
  if(regallocate<20)
  return regallocate++;
  else
  {
    printf("Can't assign register\n");
    exit(1);
  }

  return -1;
}

void freeReg(){
  if(regallocate>3)
  regallocate--;
  return ;
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

void gstinstall(char *name, struct ntype *ttype, int size, struct parameterlist *plist)
{
  struct globalsymboltable *newgst = (struct globalsymboltable*) malloc(sizeof(struct globalsymboltable));
  newgst->name=(char*)malloc(sizeof(char*));
  newgst->name=name;
  newgst->type=ttype;
  newgst->size=size;
  newgst->plist=plist;
  if(size!=0) //if variable
  {
  newgst->binding=staticmem;
  staticmem=staticmem+size;
  newgst->functionlabel=0;
  }
  else //if function
  {
    newgst->binding=0;
    newgst->functionlabel=getLabel();
  }

  newgst->nextentry=NULL;

  if(gsthead==NULL)
  {
    gsthead=newgst;
    return;
  }
  struct globalsymboltable *trv=gsthead;
  while(trv->nextentry!=NULL)
  {
    trv=trv->nextentry;
  }
  trv->nextentry=newgst;
  return;
}

struct globalsymboltable* gstlookup(char *name)
{
  struct globalsymboltable *trv=gsthead;
  if(gsthead==NULL)
  return NULL;

  while(strcmp(trv->name,name)!=0)
  {
    trv=trv->nextentry;
    if(trv==NULL)
    return NULL;
  }
  return trv;
}


struct localsymboltable* lstlookup(char *name)
{
  struct localsymboltable *sttemp=lsthead;
  if(lsthead==NULL)
  return NULL;

  while(strcmp(sttemp->name,name)!=0)
  {
    sttemp=sttemp->nextentry;
    if(sttemp==NULL)
    return NULL;
  }
  return sttemp;
}

void lstinstall(char *name, struct ntype *ttype)
{
  struct localsymboltable *sttemp=(struct localsymboltable*)malloc(sizeof(struct localsymboltable));
  sttemp->name=(char*)malloc(sizeof(char*));
  sttemp->name=name;
  sttemp->type=ttype;
  sttemp->size=ttype->width*ttype->arraysize;
  sttemp->nextentry=NULL;
  sttemp->binding=localbinding;
  localbinding=localbinding+sttemp->size;
  if(lsthead==NULL)
  {
    lsthead=sttemp;
    return ;
  }
  struct localsymboltable *trv=lsthead;
  while(trv->nextentry!=NULL)
  {
    trv=trv->nextentry;
  }
  trv->nextentry=sttemp;
  return ;
}

void printglobalsymboltable()
{
  struct globalsymboltable *trv=gsthead;
  printf("\nThe Global Symbol Table \n");
  while(trv!=NULL)
  {
    if(trv->functionlabel==0) //its a global variable
    {
      printf("Variable ");
      printf("%s ",trv->name);
      printf("%d ",trv->type->vartype );
      printf("%d ",trv->size);
      printf("%d ",trv->binding );
    }
    else
    {
      printf("function\n");
      printf("%s ",trv->name);
      printf("%d ",trv->type->vartype );
      printf("%d ",trv->functionlabel );
      struct parameterlist *ptrv=trv->plist;
      while(ptrv!=NULL)
      {
        printf("%s ",ptrv->name );
        printf("%d ",ptrv->type->vartype);
        ptrv=ptrv->nextentry;
      }
    }
    trv=trv->nextentry;
    printf("\n");
  }
}

void printlocalsymboltable()
{
  struct localsymboltable *trv=lsthead;
  printf("Symboltable\n");
  while(trv!=NULL)
  {
    printf("%s, %d, %d, %d, %d\n",trv->name,trv->type->vartype,trv->size,trv->type->width,trv->binding );
    trv=trv->nextentry;
  }
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

    case 2: //return variable in register
    p=getReg();
    if(stores==1)
    {

      if(t->lsymtab!=NULL)
      {
      fprintf(fp, "MOV R%d, BP\n",p);
      l= t->lsymtab->binding+t->val;
      fprintf(fp, "ADD R%d, %d\n",p,l);

      }
      else
      {
        l=t->gsymtab->binding+t->val;
        fprintf(fp, "MOV R%d, %d\n",p,l );

      }

    }
    else
    {
    if(t->lsymtab!=NULL)
    {
      fprintf(fp, "MOV R%d, BP\n",p);
      l= t->lsymtab->binding+t->val;
      fprintf(fp, "ADD R%d, %d\n",p,l);
      fprintf(fp, "MOV R%d,[R%d]\n",p,p);
    }
    else
    {
      l=t->gsymtab->binding+t->val;
      fprintf(fp, "MOV R%d, [%d]\n",p,l);
    }
    }
    return p;
    break;

    case 3: //code for read
    fprintf(fp, "MOV R0,\"Read\"\nPUSH R0\nMOV R0,-1\nPUSH R0\n");
    if(t->left->nodetype==2)
    {
    if(t->left->lsymtab!=NULL)
    a=t->left->lsymtab->binding+(t->left->val);
    else
    a=t->left->gsymtab->binding+(t->left->val);
    stores=1;
    a=codegen(t->left);
    stores=0;
    fprintf(fp, "MOV R0, R%d\n",a);
    }
    else
    {
      stores=1;
      a=codegen(t->left);
      stores=0;
      fprintf(fp, "MOV R0, R%d\n",a);
    }

    fprintf(fp, "PUSH R0\nPUSH R0\nPUSH R0\nCALL 0\nPOP R0\nPOP R1\nPOP R1\nPOP R1\nPOP R1\n");
    return 1;
    break;

    case 4://code for write
    fprintf(fp, "MOV R2, \"Write\"\nPUSH R2\n");
    p=codegen(t->left);
    //fprintf(fp, "MOV R%d, [R%d]\n",p,p);
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
      stores=1;
      a=codegen(t->left);
      stores=0;
      fprintf(fp, "MOV [R%d],R%d\n",a,l);
      freeReg();
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

    case 11: //array [i]
    if(stores==1||stores==0)
    {
      stores=stores+10;
    if(t->left->nodetype==2)
    {
    r=codegen(t->right);
    if(t->left->lsymtab!=NULL)
    l=t->left->lsymtab->binding;
    else
    l=t->left->gsymtab->binding;

    fprintf(fp, "MUL R%d, %d\n",r,t->val );
    fprintf(fp, "ADD R%d, %d\n",r,l);
    stores=stores-10;
    if(stores==0)
    fprintf(fp, "MOV R%d, [R%d]\n",r,r);
    return r;
    }
    else {
    l=codegen(t->left);
    r=codegen(t->right);
    fprintf(fp , "MUL R%d, %d\n", r,t->val);
    fprintf(fp, "ADD R%d, R%d\n",r,l);
    stores=stores-10;
    if(stores==0)
    fprintf(fp, "MOV R%d, [R%d]\n",r,r);
    return r;
    }
  }
  if(stores>9)
  {
    if(t->left->nodetype==2)
    {
    r=codegen(t->right);
    if(t->left->lsymtab!=NULL)
    l=t->left->lsymtab->binding;
    else
    l=t->left->gsymtab->binding;
    fprintf(fp, "MUL R%d, %d\n",r,t->val );
    fprintf(fp, "ADD R%d, %d\n",r,l);
    return r;
    }
    else {
    l=codegen(t->left);
    r=codegen(t->right);
    fprintf(fp , "MUL R%d, %d\n", r,t->val);
    fprintf(fp, "ADD R%d, R%d\n",r,l);
    return r;
    }
  }
    break;

    case 12: //return statements
    if(mainfunction==0) //normal functions
    {
      l=codegen(t->left);
      p=getReg();
      fprintf(fp, "MOV R%d, BP\n",p );
      fprintf(fp, "SUB R%d, 2\n",p );
      fprintf(fp, "MOV [R%d],R%d\n",p,l );
      freeReg();
      freeReg();
      fprintf(fp, "MOV SP, BP\n");
      //fprintf(fp, "SUB SP, 1\n");
      fprintf(fp, "POP BP\n");
      fprintf(fp, "RET\n");
    }
    else //mainfunction
    {
      fprintf(fp,"MOV R2,\"Exit\"\nPUSH R2\nPUSH R2\n");
      fprintf(fp,"PUSH R2\nPUSH R2\nPUSH R2\nCALL 0\nRET");
      fclose(fp);
    }
    return 1;
    break;

    case 13://function call statements
    l=0;
    p=getReg();
    freeReg();
    while(l<p)
    {
      fprintf(fp, "PUSH R%d\n",l );
      l++;
    }

    //evaluate arguments
    struct tnode *temp=t->left;
    while(temp->right!=NULL&&temp->nodetype==14)
    {
      l=codegen(temp->right);
      fprintf(fp, "PUSH R%d\n",l);
      temp=temp->left;
    }
    l=codegen(temp);
    fprintf(fp, "PUSH R%d\n",l);
    fprintf(fp, "PUSH R0\n");
    fprintf(fp, "CALL L%d\n",t->gsymtab->functionlabel);
    r=getReg();//return value
    fprintf(fp, "POP R%d\n",r );
    temp=t->left;
    while(temp->right!=NULL&&temp->nodetype==14)
    {
      fprintf(fp, "POP R0\n");
      temp=temp->left;
    }
    fprintf(fp, "POP R0\n");
    l=p-1;
    while(l>=0)
    {
      if(l!=r)
      fprintf(fp, "POP R%d\n",l );
      l--;
    }
    return r;
    break;
    case 14:
    break;
    default:
    break;


  }


}
