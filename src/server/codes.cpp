#include <stdio.h>
#include "codes.h"
#include "assert.h"
#define MIN(a,b) (a<b ? a : b)

// HOLGER 14Jun06: COPY FROM codes.h

// ######################################################
inline int Is_Pow2(unsigned int n)  
{if (n && !(n & (n - 1))) return(1); else return(0);}

inline int floor_log2(unsigned int n)
{ int l=0;
  if (!n) return(-1); n=n>>1;
  while(n) {n=n>>1; l++;}
  return(l);
}

// ######################################################
inline int unary_dec(unsigned int *code,unsigned int &bp)
{ // return number of 0 before next 1
   int l=-(bp & 0x1F);
   int wp=(bp>>5);
   unsigned int v=code[wp++] & (0xFFFFFFFF << (bp & 0x1F));

   while (!v) {l+=32; v=code[wp++];}
   if (!(v<<16)) {v=v>>16; l+=16;}
   if (!(v<<24)) {v=v>> 8; l+= 8;}
   if (!(v<<28)) {v=v>> 4; l+= 4;}
   if (!(v<<30)) {v=v>> 2; l+= 2;}
   if (!(v<<31)) {v=v>> 1; l+= 1;}
   bp+=l+1;
   return(l);
 }

// ######################################################
inline void unary_enc(unsigned int n,unsigned int *code,unsigned int &bp)
{ // encode n unary
  unsigned int wp=(bp>>5); /* Holger 11Jan06, was:  int wp=(bp>>5); */
  code[wp++]&= (1<<(bp & 0x1F))-1; // zero all bits in current word with address >=bp
  bp+=n;
  while (wp<=(bp>>5)) code[wp++]=0;
  code[(bp>>5)] |= (1<<(bp & 0x1F));
  bp++;
}

// ######################################################
inline unsigned int Elias_Gamma_dec1(unsigned int *code,unsigned int &bp)
{
  unsigned int l=(unsigned int)unary_dec(code,bp); /* Holger 11Jan06, was:  int l=unary_dec(code,bp); */
  if (!l) return(1);
  if (l>31) {printf("Number >= 2^32\n"); return(0);}

  unsigned int v=code[bp>>5]>>(bp&0x1F);
  if ((32 - (bp & 0x1F)) < l) v |= (code[(bp>>5)+1] << (32u - (bp&0x1F)));
  bp=bp+l; 
  v = (1<<l) | (v & ((1<<l)-1));
  return(v);
}
// ###################################################################
inline unsigned int Elias_Gamma_enc1(unsigned int n,unsigned int *code,unsigned int bp)
{
  if (n==0) { printf("Elias cannot encode 0\n"); return(0);}
  int l=floor_log2(n);
  code[(bp>>5)]&= (1<<(bp & 0x1F))-1;
  code[(bp>>5)+1]=0;
  
  bp+=l;
  n=(n<<1)+1;
  code[bp>>5] |= n<<(bp & 0x1F);
  code[(bp>>5)+1]= n >> (32-(bp & 0x1F));
  return(bp+l+1);
}
// ###################################################################
inline unsigned int Elias_Delta_enc1(unsigned int n,unsigned int *code,unsigned int bp)
{
  if (n==0) { printf("Elias cannot encode 0\n"); return(0);}
  int l=floor_log2(n);
  bp = Elias_Gamma_enc1(l+1,code,bp);
 
  code[bp>>5] &=(1<<(bp & 0x1F))-1;
  code[bp>>5] |= n<<(bp & 0x1F);
  code[(bp>>5)+1]= n >> (32-(bp & 0x1F));
  return(bp+l);
}
// ######################################################
inline int Elias_Delta_dec1(unsigned int *code,unsigned int &bp)
{
  unsigned int p = bp;
  unsigned int l=(unsigned int)Elias_Gamma_dec1(code,bp)-1; /* Holger 11Jan06, was:  int l=Elias_Gamma_dec1(code,bp)-1; */
  if (!l) return(1);
  if (l>31)  {bp=p; return(0);}

  unsigned int v=code[bp>>5]>>(bp&0x1F);
  if ((32 - (bp & 0x1F)) <l)  v |= (code[(bp>>5)+1]<< (32 - (bp&0x1F)));
  bp=bp+l; 
  v = (1<<l) | (v & ((1<<l)-1));
  return(v);
}


// END COPY FROM codes.h 



// #########################################################################
const int Simple9_MaxValue[9]={0x0FFFFFFF,0x3FFF,0x1FF,0x7F,0x1F,0xF,0x7,0x3,0x1};
const int Simple9_MaxCodes[9]={1,2,3,4,5,7,9,14,28};
const int Simple9_CodeLeng[9]={28,14,9,7,5,4,3,2,1};

// #########################################################################
int Simple9_enc(unsigned int n,const int * cleartext,unsigned int *code)
{
  int cs,csi,i,next=0,pos=0;
  unsigned int codeword;

  while (next<n)
    {
      // determine the coding scheme
      if (cleartext[next]>Simple9_MaxValue[0]) 
	{printf("cannot encode %i with 28 bits\n",cleartext[next]); return(0);}
      cs=8;
      for (csi=1;csi<9;csi++)
	for (i=next;i<MIN(next+Simple9_MaxCodes[csi],n);i++)
	  if (cleartext[i]>Simple9_MaxValue[csi]) {cs=csi-1; csi=20; break;} // use codescheme cs
      
      codeword=0;
      for (i=MIN(next+Simple9_MaxCodes[cs],n)-1;i>=next;i--)
	codeword=(codeword<<Simple9_CodeLeng[cs])|cleartext[i];
      codeword=codeword|(cs<<28);
      next=MIN(next+Simple9_MaxCodes[cs],n);
      code[pos++]=codeword;
    }
  return(pos);
}
// #########################################################################
int Simple9_dec(unsigned int n,int * cleartext, const unsigned int *code)
{
  register int cs,csi,i,next=0,pos=0;
  register unsigned int codeword;

  while (next<n)
    {
      codeword=code[pos++];
      cs=codeword>>28;
      for (i=0;i<MIN(Simple9_MaxCodes[cs],n-next);i++)
	{
	  cleartext[next+i]=codeword & Simple9_MaxValue[cs];
	  codeword=codeword>>Simple9_CodeLeng[cs];
	}

      next+=Simple9_MaxCodes[cs];
    }
  return pos;
}

// #########################################################################
void Simple9_dec_gaps_with_boundaries(unsigned int n,int * cleartext, const unsigned int *code)
{
  register int cs,csi,i,j=0,next=0,pos=0;
  register unsigned int codeword,curr,abs=0;
  
    while (next<n)
    {
      codeword=code[pos++];
      cs=codeword>>28;
      for (i=0;(i<MIN(Simple9_MaxCodes[cs],n-next))&&(j<n);i++)
	{
	  curr = codeword & Simple9_MaxValue[cs];
	  if(abs==0)//last number was a zero
	    {
	      //     assert((j==0)||(curr<= cleartext[j-1]));// 0 can occur!
	      assert(curr>0);
	      cleartext[j] = (curr-1);
	      ++j;
	      // abs += (curr-1);// could be zero
	      abs += curr;// one too large !
	    }
	  else if( curr != 0)
	    {
	      assert(curr>0);
	      assert(j>0);
	      assert(abs>0);
	      abs += (curr-1);
	      assert(abs-1 >= cleartext[j-1]);
	      cleartext[j] = abs-1;
	      ++j;
	    }
	  else
	    {
	      assert(abs!=0);
	      assert(curr==0);
	      abs = 0;
	    }	    
	  codeword=codeword>>Simple9_CodeLeng[cs];
	}

      next+=Simple9_MaxCodes[cs];
    }
}

// #########################################################################
void Simple9_dec_gaps(unsigned int n,int * cleartext, const unsigned int *code)
{
  int cs,csi,i,next=0,pos=0;
  unsigned int codeword,abs=0;

  while (next<n)
    {
      codeword=code[pos++];
      cs=codeword>>28;
      for (i=0;i<MIN(Simple9_MaxCodes[cs],n-next);i++)
	{
	  abs += codeword & Simple9_MaxValue[cs];
	  cleartext[next+i] = abs;
	  codeword=codeword>>Simple9_CodeLeng[cs];
	}

      next+=Simple9_MaxCodes[cs];
    }
}

// #########################################################################
void Byte_Aligned_dec(unsigned int n,int * cleartext, const unsigned int *code)
{
  int next=0;
  int pos=0;
  int s=0;
  unsigned int ct=0;
  
  while (next<n)
    {
      unsigned int w=code[pos++];
      
      ct=ct | ((w&0x7F)<<s); s=s+7;
      if (w & 0x80) {cleartext[next++]=ct; ct=s=0;}
     
      w=w>>8;
      ct=ct | ((w&0x7F)<<s); s=s+7;
      if (w & 0x80) {cleartext[next++]=ct; ct=s=0;}

      w=w>>8;
      ct=ct | ((w&0x7F)<<s); s=s+7;
      if (w & 0x80) {cleartext[next++]=ct; ct=s=0;}

      w=w>>8;
      ct=ct | ((w&0x7F)<<s); s=s+7;
      if (w & 0x80) {cleartext[next++]=ct;  ct=s=0;}
    }
}
// #########################################################################
int Byte_Aligned_enc(unsigned int n,int * cleartext,unsigned int *code)
{
  int next=0;
  char *pos=(char *)code;
  char c;

  while (next<n)
    {
      unsigned int w=cleartext[next++];

      c=(char) (w & 0x7F);
      w=w>>7;
      if (w==0) {*pos++ = (c | 0x80);  continue;}
      else *pos++ = c ;

      c=(char) (w & 0x7F);
      w=w>>7;
      if (w==0) {*pos++ = (c | 0x80);  continue;}
      else *pos++ = c;

      c=(char) (w & 0x7F);
      w=w>>7;
      if (w==0) {*pos++ = (c | 0x80);  continue;}
      else *pos++ = c ;

      c=(char) (w & 0x7F);
      w=w>>7;
      if (w==0) *pos++ = (c | 0x80);
      else {printf("cannot encode %i with 28 bits\n",cleartext[next-1]); return(0);}
    }
  return (int)((unsigned int*)(pos) - code);
  // return((int) pos - (int) code);
}

// ######################################################
void Elias_Delta_dec(unsigned int n,int * cleartext,unsigned int *code)
{
  unsigned int i,bp;
  for (bp=i=0;i<n;i++) cleartext[i]=Elias_Delta_dec1(code,bp);
}
// ######################################################
int Elias_Delta_enc(unsigned int n,int * cleartext,unsigned int *code)
{
  unsigned int i,bp;
  for (bp=i=0;i<n;i++) bp=Elias_Delta_enc1(cleartext[i],code,bp);
  return(bp);
}
// ######################################################
void Elias_Gamma_dec(unsigned int n,int * cleartext,unsigned int *code)
{
  unsigned int i,bp;
  for (bp=i=0;i<n;i++) cleartext[i]=Elias_Gamma_dec1(code,bp);
}
// ######################################################
int Elias_Gamma_enc(unsigned int n,int * cleartext,unsigned int *code)
{
  unsigned int i,bp;
  for (bp=i=0;i<n;i++) bp=Elias_Gamma_enc1(cleartext[i],code,bp);
  return(bp);
}
// #########################################################################
int Golomb_unary_enc(unsigned int n,int * cleartext,unsigned int *code,int q)
{
  int log_q =floor_log2(q);
  unsigned int toggle=(1<<(log_q+1))-q;
  unsigned int bp=0;

  for (int i=0;i<n;i++)
    {
      int g=cleartext[i] / q;
      int r=cleartext[i] % q;

      unary_enc(g,code,bp);

      code[bp>>5]   &= (1<<     (bp & 0x1F))-1;
      unsigned int l=log_q;
      if (r>=toggle) {l=r+toggle; r = ((l&1)<<log_q) | (l>>1); l=log_q+1; }
      code[bp>>5]   |= r <<     (bp & 0x1F);
      code[(bp>>5)+1]= r >> (32-(bp & 0x1F));
      bp+=l;
    }
  return(bp);
}
// #########################################################################
int Golomb_Gamma_enc(unsigned int n,int * cleartext,unsigned int *code,int q)
{
  int log_q =floor_log2(q);
  unsigned int toggle=(1<<(log_q+1))-q;
  unsigned int bp=0;

  for (int i=0;i<n;i++)
    {
      int g=cleartext[i] / q;
      int r=cleartext[i] % q;

      bp=Elias_Gamma_enc1(g+1,code,bp);

      code[bp>>5]   &= (1<<     (bp & 0x1F))-1;
      unsigned int l=log_q;
      if (r>=toggle) {l=r+toggle; r = ((l&1)<<log_q) | (l>>1); l=log_q+1; }
 
      code[bp>>5]   |= r <<     (bp & 0x1F);
      code[(bp>>5)+1]= r >> (32-(bp & 0x1F));
      bp+=l;
    }
  return(bp);
}

// ###################################################################
void  Golomb_unary_dec(unsigned int n,int * cleartext,unsigned int *code,int q)
{
  int log_q =floor_log2(q);
  unsigned int toggle=(1<<(log_q+1))-q;
  unsigned int bp=0;
  unsigned int v,l;

  for (int i=0;i<n;i++)
    {
      cleartext[i]=q*unary_dec(code,bp);

      l=bp & 0x1F;
      v=code[bp>>5]>>l;
      if (l) v |= (code[(bp>>5)+1]<< (32 - l));
     
      bp+=log_q;
      l= v & ((1<<log_q)-1);
      if ( l >= toggle) {bp++; l = (l<<1)-toggle + ((v>>log_q)&1); }
      cleartext[i]+=l;
    }
}



// ######################################################
void  Golomb_Gamma_dec(unsigned int n,int * cleartext,unsigned int *code,int q)
{
  int log_q =floor_log2(q);
  unsigned int toggle=(1<<(log_q+1))-q;
  unsigned int bp=0;
  unsigned int v,l;

  for (int i=0;i<n;i++)
    {
      cleartext[i]=q*(Elias_Gamma_dec1(code,bp)-1);

      l=bp & 0x1F;
      v=code[bp>>5]>>l;
      if (l) v |= (code[(bp>>5)+1]<< (32 - l));
     
      bp+=log_q;
      l= v & ((1<<log_q)-1);
      if ( l >= toggle) {bp++; l = (l<<1)-toggle + ((v>>log_q)&1); }
      cleartext[i]+=l;
    }
}
