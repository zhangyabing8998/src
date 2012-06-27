/* fast 3D dip estimation by plane wave destruction */


#include <rsf.h>


static int n,n1,n2,n3,n1x3,n12x3;
static float *u1,*num,*den;

static void fdip_flt1(float *in,float *out);
static void fdip_dim2(const float *in,float *out);
static void fdip_dim3(const float *in,float *out);
static void quadratic(float aa, float bb, float cc, 
		      float *num, float *den);

void fdip_init(int m1,int m2,int m3,int *rect, int niter, bool verb)
/*< initialize >*/
{
	int nn[3];
	n1=m1;
	n2=m2;
	n3=m3;

	n=n1*n2*n3;
	n1x3=n1*3;
	n12x3=n1*n2*3;

	nn[0]=n1;
	nn[1]=n2;
	nn[2]=n3;

	u1=sf_floatalloc(3*n);
	num=sf_floatalloc(n);
	den=sf_floatalloc(n);

	if(n3 !=1 )
		sf_divn_init (3, n, nn, rect, niter, verb);
	else sf_divn_init (2, n, nn, rect, niter, verb);
}

void fdip_close()
/*< release work space >*/
{
	free(u1);
	free(num);
	free(den);
	sf_divn_close();
}


void fdip(float *in,float *out,
	int dim /* 0 - inline; 1 - xline; X - both*/)
/*< 3D dip estimation >*/
{	
	int n23,i23;

	n23=n2*n3;
	for(i23=0;i23<n23;i23++)
		fdip_flt1(in+n1*i23,u1+n1x3*i23);
	
	if(dim != 1) // need inline
	{
		fdip_dim2(u1,out);
		out += n;
	}

	if(dim != 0) // need xline
	{
		fdip_dim3(u1,out);
	}
}

static void fdip_flt1(float *in,float *out)
// filter in time direction
{
	int i1;

	// 0 boundary
//	*out++ = in[1] - 2.0*in[0];
//	*out++ = 1.5*in[1];
//	*out++ = 8.0*in[0] + 2.0*in[1];
	*out++ = 0.0;
	*out++ = 0.0;
	*out++ = 0.0;
	in++;
	for(i1=1;i1<n1-1;i1++,in++)
	{
		*out++ = in[-1] + in[1] - 2.0*in[0];
		*out++ = 1.5 * (in[1] - in[-1]);
		*out++ = 2.0* (in[-1] + 4.0*in[0] + in[1]);
	}
	// n1 boundary
	*out++ = 0.0;
	*out++ = 0.0;
	*out++ = 0.0;
//	*out++ = in[-1] - 2.0*in[0];
//	*out++ = -1.5*in[-1];
//	*out++ = 8.0*in[0] + 2.0*in[-1];
	in++;
}

static void fdip_dim2(const float *in,float *out)
{
	int i1,i2,i3,i123;
	float a,b,c,*p2,*p3,norm;
	p2=num;
	p3=den;
	for(i3=0;i3<n3;i3++)
	{
		for(i2=0;i2<n2-1;i2++)
		{
			for(i1=0;i1<n1;i1++)
			{
				a = in[n1x3] - in[0];	in++;
				b = in[n1x3] + in[0];	in++;
				c = in[n1x3] - in[0];	in++;
				quadratic(a,b,c, p2++, p3++);
			}
		}
		for(i1=0;i1<n1;i1++)
		{
			*p2=p2[-n1x3];	p2++;
			*p3=p3[-n1x3];	p3++;
		}
		in += n1x3;
	}

	norm=0.0;
	for (i123=0;i123<n;i123++)
		norm += den[i123]*den[i123];
	norm = sqrt(n/norm);

	for (i123=0;i123<n;i123++)
	{
		num[i123] *= norm;
		den[i123] *= norm;
	}

	sf_divn (num, den, out);
}

static void fdip_dim3(const float *in,float *out)
{
	int i1,i2,i3,i123;
	float a,b,c,*p2,*p3,norm;
	p2=num;
	p3=den;
	for(i3=0;i3<n3-1;i3++)
	{
		for(i2=0;i2<n2;i2++)
		{
			for(i1=0;i1<n1;i1++)
			{
				a = in[n12x3] - in[0];	in++;
				b = in[n12x3] + in[0];	in++;
				c = in[n12x3] - in[0];	in++;
				quadratic(a,b,c, p2++, p3++);
			}
		}
	}
	for(i2=0;i2<n2;i2++)
	{
		for(i1=0;i1<n1;i1++)
		{
			*p2=p2[-n12x3];	p2++;
			*p3=p3[-n12x3];	p3++;
		}
	}

	norm=0.0;
	for (i123=0;i123<n;i123++)
		norm += den[i123]*den[i123];
	norm = sqrt(n/norm);

	for (i123=0;i123<n;i123++)
	{
		num[i123] *= norm;
		den[i123] *= norm;
	}


	sf_divn (num, den, out);
}

static void quadratic(float aa, float bb, float cc, 
		      float *num, float *den)
/* solution of a*x^2+2*b*x+c=0 is num/den */
{
    float d;

    d = bb*bb-aa*cc;
    
    if (d < 0.) {
	*num=-bb;
	*den=aa;
	return;
    }

    d = sqrtf(d);

    if (bb > 0) {
	*den = bb+d;
    } else {
	*den = bb-d;
    }
	
	*num = -cc;
}

