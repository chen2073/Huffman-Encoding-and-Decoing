#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#define NUMLETTER 128
#define TEXT 1
#define BINARY 2
#define QUEUE  0
#define STACK  1
#define SORTED 2

typedef struct
{
  char value;
  int freq;
} CharFreq;

typedef struct treenode
{
  struct treenode * left;
  struct treenode * right;
  char value; 
  int freq;  
} TreeNode;

TreeNode * TreeNode_create(char val, int freq)
{
  TreeNode * tn = malloc(sizeof(TreeNode));
  tn -> left = NULL;
  tn -> right = NULL;
  tn -> value = val;
  tn -> freq = freq;
  return tn;
}

TreeNode * Tree_merge(TreeNode * tn1, TreeNode * tn2)
{
  TreeNode * tn = malloc(sizeof(TreeNode));
  tn -> left = tn1;
  tn -> right = tn2;
  tn -> value = 0; 
  tn -> freq = tn1 -> freq + tn2 -> freq;
  return tn;
}

typedef struct listnode
{
  struct listnode * next;
  TreeNode * tnptr;
} ListNode;

ListNode * ListNode_create(TreeNode * tn)
{
  ListNode * ln = malloc(sizeof(ListNode));
  ln -> next = NULL;
  ln -> tnptr = tn;
  return ln;
}

ListNode * List_insert(ListNode * head, ListNode * ln, int mode)
{
  if (head == NULL)
    {
      return ln;
    }
  if (mode == STACK)
    {
      ln -> next = head;
      return ln;
    }
  if (mode == QUEUE)
    {
      head -> next = List_insert(head -> next, ln, mode);
      return head;
    }

  int freq1 = (head -> tnptr) -> freq;
  int freq2 = (ln -> tnptr) -> freq;
  if (freq1 > freq2) 
    {
      ln -> next = head;
      return ln;
    }

  head -> next = List_insert(head -> next, ln, mode);
  return head;
}

ListNode * List_build(CharFreq * frequencies)
{
  
  int ind = 0;
  while (frequencies[ind].freq == 0)
    {
      ind ++;
    }
  if (ind == NUMLETTER) 
    {
      return NULL;
    }
  
  ListNode * head = NULL;
  while (ind < NUMLETTER)
    {
      TreeNode * tn = TreeNode_create((frequencies[ind].value), (frequencies[ind].freq));
      ListNode * ln = ListNode_create(tn);
      head = List_insert(head, ln, SORTED);
      ind ++;
    }
  return head;
}

static ListNode * MergeListNode(ListNode * head)
{
  ListNode * second = head -> next;
  ListNode * third  = second -> next;
  TreeNode * tn1 = head -> tnptr;
  TreeNode * tn2 = second -> tnptr;
  
  free (head);
  free (second);
  head = third;
  TreeNode * mrg;

  mrg = Tree_merge(tn2, tn1);
  ListNode * ln = ListNode_create(mrg);
  head = List_insert(head, ln, STACK);
 
  return head;
}

int writeBit(FILE * fptr, unsigned char bit, unsigned char * whichbit, unsigned char * curbyte)
{
  if ((* whichbit) == 0)
    {
      * curbyte = 0;
    }
  unsigned char temp = bit << (7 - (* whichbit));
  * curbyte |= temp; 
  int value = 0;
  if ((* whichbit) == 7)
    {
      int ret;
      ret = fwrite(curbyte, sizeof(unsigned char), 1, fptr);
      if (ret == 1)
	{
	  value = 1;
	}
      else
	{
	  value = -1;
	}
    }
  * whichbit = ((* whichbit) + 1) % 8;
  return value;
}

static void char2bits(FILE * outfptr, int ch, unsigned char * whichbit, unsigned char * curbyte)
{
  unsigned char mask = 0x40; 
  while (mask > 0)
    {
      writeBit(outfptr, (ch & mask) == mask,
	       whichbit, curbyte);
      mask >>= 1;
    }
}

int padZero(FILE * fptr, unsigned char * whichbit, unsigned char * curbyte)
{
  int rtv;
  while ((* whichbit) != 0)
    {
      rtv = writeBit(fptr, 0, whichbit, curbyte);
      if (rtv == -1)
	{
	  return -1;
	}
    }
  return rtv;
}

static void Tree_leafHelper(TreeNode * tn, int * num)
{
  if (tn == 0)
    {
      return;
    }
  TreeNode * lc = tn -> left;
  TreeNode * rc = tn -> right;
  if ((lc == NULL) && (rc == NULL))
    {
      (* num) ++;
      return;
    }
  Tree_leafHelper(lc, num);
  Tree_leafHelper(rc, num);
}

int Tree_leaf(TreeNode * tn)
{
  int num = 0;
  Tree_leafHelper(tn, & num);
  return num;
}

static int Tree_heightHelper(TreeNode * tn, int height)
{
  if (tn == 0)
    {
      return height;
    }
  int lh = Tree_heightHelper(tn -> left, height + 1);
  int rh = Tree_heightHelper(tn -> right, height + 1);
  if (lh < rh)
    { 
      return rh;
    }
  if (lh > rh)
    { 
      return lh;
    }
  return lh;
}

int Tree_height(TreeNode * tn)
{
  return Tree_heightHelper(tn, 0);
}

int countFrequency(char * filename, CharFreq * freq)
{
  FILE * fptr = fopen(filename, "r");
  int count = 0;
  while (! feof (fptr))
    {
      int onechar = fgetc(fptr);
      if (onechar != EOF)
	{
	  count ++;
	  freq[onechar].value = (char) onechar;
	  freq[onechar].freq ++;
	}
    }
  fclose (fptr);
  return count;
}

static void Tree_headerHelper(TreeNode * tn, FILE * outfptr, unsigned char * whichbit, unsigned char * curbyte)
{
  if (tn == NULL)
    {
      return; 
    }
  TreeNode * lc = tn -> left;
  TreeNode * rc = tn -> right;
  if ((lc == NULL) && (rc == NULL))
    {
      writeBit(outfptr, 1, whichbit, curbyte);
      char2bits(outfptr, tn -> value, whichbit, curbyte);
      return;
    }
  Tree_headerHelper(lc, outfptr, whichbit, curbyte);
  Tree_headerHelper(rc, outfptr, whichbit, curbyte);
  writeBit(outfptr, 0, whichbit, curbyte);
}
void Tree_header(TreeNode * tn, unsigned int numChar, char * outfile)
{
  FILE * outfptr = fopen(outfile, "w");
  if (outfptr == NULL)
    {
      return;
    }
  unsigned char whichbit = 0;
  unsigned char curbyte = 0;
  Tree_headerHelper(tn, outfptr, & whichbit, & curbyte);
  writeBit(outfptr, 0, & whichbit, & curbyte);
  padZero(outfptr, & whichbit, & curbyte);

  fwrite(& numChar, sizeof(unsigned int), 1, outfptr);
  unsigned char newline = '\n';
  fwrite(& newline, sizeof(unsigned char), 1, outfptr);
  fclose (outfptr);
}

void Tree_destroy(TreeNode * tn)
{
  if (tn == NULL)
    {
      return;
    }
  Tree_destroy(tn -> left);
  Tree_destroy(tn -> right);
  free (tn);
}

int readBit(FILE * fptr, unsigned char * bit, unsigned char * whichbit, unsigned char * curbyte)
  
{
  int ret = 1;
  if ((* whichbit) == 0)
    {
      ret = fread(curbyte, sizeof(unsigned char), 1, fptr);
    }
  if (ret != 1)
    {
      return -1;
    }
  unsigned char temp = (* curbyte) >> (7 - (* whichbit));
  temp = temp & 0X01; 
  * whichbit = ((* whichbit) + 1) % 8;
  * bit = temp;
  return 1;
}

static TreeNode * readHeader(FILE * infptr)
{
  int done = 0;
  unsigned char whichbit = 0;
  unsigned char curbyte  = 0;
  unsigned char onebit   = 0;
  ListNode * head = NULL;
  
  while (done == 0)
    {
      readBit(infptr, & onebit, & whichbit, & curbyte);
      if (onebit == 1)
	{
	  int bitcount;
	  unsigned char value = 0;
	  for (bitcount = 0; bitcount < 7; bitcount ++)
	    {
	      value <<= 1; 
	      readBit(infptr, & onebit, & whichbit, & curbyte);
	      value |= onebit;
	    }
	  TreeNode * tn = TreeNode_create(value, 0); 
	  ListNode * ln = ListNode_create(tn);
	  head = List_insert(head, ln, STACK);
	}
      else
	{
	  if ((head -> next) == NULL)
	    {
	      done = 1;
	    }
	  else
	    {
	      head = MergeListNode(head);
	    }
	}
    }
  
  TreeNode * root = head -> tnptr;
  free (head);
  return root;
}

int decode(char * infile, char * outfile)
{
  FILE * infptr = fopen(infile, "r");
  TreeNode * root = readHeader(infptr);

 
  unsigned int numChar = 0;
  fread(& numChar, sizeof(unsigned int), 1, infptr);
 
  unsigned char newline;
  fread(& newline, sizeof(unsigned char), 1, infptr);
  
  unsigned char whichbit = 0;
  unsigned char onebit = 0;      
  unsigned char curbyte = 0;
  FILE * outfptr = fopen(outfile, "w");
  while (numChar != 0)
    {
      TreeNode * tn = root;
      while ((tn -> left) != NULL)
	{
	  readBit(infptr, & onebit, & whichbit, & curbyte);	  
	  if (onebit == 0)
	    {
	      tn = tn -> left;
	    }
	  else
	    {
	      tn = tn -> right;
	    }	    
	}							
      fprintf(outfptr, "%c", tn -> value);
      numChar --;
    }
  Tree_destroy(root);
  fclose(infptr);
  fclose(outfptr);
  return 1;
}

int main(int argc, char ** argv)
{
	
	int len = strlen(argv[1]);
	char out[len+7];
	
	strcpy(out, argv[1]);
	strcat(out, ".unhuff");
	decode(argv[1], out);

return 0;
}
