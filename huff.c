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

  mrg = Tree_merge(tn1, tn2);
  ListNode * ln = ListNode_create(mrg);
  head = List_insert(head, ln, SORTED);
 
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

static TreeNode * list2Tree(ListNode * head)
{
  while ((head -> next) != NULL)
    {
      head = MergeListNode(head);
    }

  TreeNode * root = head -> tnptr;

  free (head);
  return root;
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

static int compareFreq(const void * p1, const void * p2)
{
  const CharFreq * ip1 = (const CharFreq *) p1;
  const CharFreq * ip2 = (const CharFreq *) p2;
  const int iv1 = ip1 -> freq;
  const int iv2 = ip2 -> freq;
  return (iv1 - iv2);
}

void sortFrequency(CharFreq * freq)
{
  qsort(freq, NUMLETTER, sizeof(CharFreq), compareFreq);
}

void buildCodeBookHelper(TreeNode * tn, int * * codebook, int * row, int col)
{
  if (tn == NULL)
    {
      return;
    }
  
  TreeNode * lc = tn -> left;
  TreeNode * rc = tn -> right;
  if ((lc == NULL) && (rc == NULL))
    {
      codebook[*row][0] = tn -> value;
      (* row) ++;
      return;
    }
  if (lc != NULL)
    {
      int numRow = Tree_leaf(lc);
      int ind;
      for (ind = * row; ind < (* row) + numRow; ind ++)
	{
	  codebook[ind][col] = 0;
	}
      buildCodeBookHelper(lc, codebook, row, col + 1);
    }
  if (rc != NULL)
    {
      int numRow = Tree_leaf(rc);
      int ind;
      for (ind = * row; ind < (* row) + numRow; ind ++)
	{
	  codebook[ind][col] = 1;
	}
      buildCodeBookHelper(rc, codebook, row, col + 1);
    }    
}
void buildCodeBook(TreeNode * root, int * * codebook)
{
  int row = 0;
  buildCodeBookHelper(root, codebook, & row, 1); 
}

int compress(char * infile, char * outfile, int * * codebook, int * mapping)
{
  FILE * infptr = fopen(infile, "r");
  FILE * outfptr = fopen(outfile, "a"); 
  
  unsigned char whichbit = 0;
  unsigned char curbyte = 0;
  while (! feof(infptr))
    {
      int onechar = fgetc(infptr);
      if (onechar != EOF)
	{
	  int ind = mapping[onechar];
	  int ind2 = 1;
	  while (codebook[ind][ind2] != -1)
	    {
	      writeBit(outfptr, (codebook[ind][ind2] == 1), & whichbit, & curbyte);
	      ind2 ++;
	    }
	}
    }
  padZero(outfptr, & whichbit, & curbyte);
  fclose(infptr);
  fclose(outfptr);
  return 1;
}

int encode(char * infile, char * outfile)
{
  CharFreq frequencies[NUMLETTER];
  bzero(frequencies, sizeof(CharFreq) * NUMLETTER);

  unsigned int numChar = countFrequency(infile, frequencies);
  sortFrequency(frequencies);
  ListNode * head = List_build(frequencies);
  TreeNode * root = list2Tree(head);
  int numRow = Tree_leaf(root);
  int numCol = Tree_height(root);
  numCol ++;

  int * * codebook = malloc(sizeof(int*) * numRow);
  int row;
  for (row = 0; row < numRow; row ++)
    {
      codebook[row] = malloc(sizeof(int) * numCol);
      int col;
      for (col = 0; col < numCol; col ++)
	{
	  codebook[row][col] = -1;
	}
    }
  buildCodeBook(root, codebook);

  int mapping[NUMLETTER];
  int ind;
  for (ind = 0; ind < NUMLETTER; ind ++)
    {
      mapping[ind] = -1; 
      int ind2;
      for (ind2 = 0; ind2 < numRow; ind2 ++)
	{
	  if (codebook[ind2][0] == ind)
	    {
	      mapping[ind] = ind2;
	    }
	}
    }

  Tree_header(root, numChar, outfile);
  compress(infile, outfile, codebook, mapping);

  for (ind = 0; ind < numRow; ind ++)
    {
      free (codebook[ind]);
    }
  free (codebook);

  Tree_destroy(root);
  return 1;
}

int main(int argc, char ** argv)
{
	int len = strlen(argv[1]);
	char out[len+5];
	
	strcpy(out, argv[1]);
	strcat(out, ".huff");

	encode(argv[1], out);

return 0;
}
