
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <thrust/device_vector.h>
#include <thrust/sort.h>
#include <stdio.h>
//head files of C++ feature
#include <map>
#include <string>

#include <windows.h> //this is related to windows platform, used to launch another program

#define PI 3.141593
#define K_SPHERE (4.0/3.0*PI)
#define grid_size (N_bubble-1)/Block_size+1 //calculate how many blocks need to be launched
#define NULLD N_max_bubble_d
#define VEC_D thrust::raw_pointer_cast(&vec_bubbles[0])
//define the data type
class Bubble
{
public:
	float x,y,z;//coordinates of the center
	float R; //radius of the bubble
	float R3; //volume, used to reduce the error
	size_t ip; //index of pointing to
	size_t iped; //index of being pointed
};
typedef Bubble* PBubble;

// user-defined comparison operator that acts like less<int>,
struct compare_height
{
	__device__
		bool operator()(PBubble x, PBubble y)
	{
		return x->z > y->z; //let the bigger ones stand in front
	}
};

//global variables in the GPU, whose initial values are obtained from CPU
__constant__ float Hm_max;//horizontal move very step, unit mm
__constant__ float K_velocity; //the coefficient in the formula u=(2*g*rho/9/eta)*R^2
__constant__ float K_escape; //determine the critical condition for the bubble to escape
__constant__ float Width_container_d; //_d fix means it's a copy from the CPU, not unique in GPU
__constant__ float Height_container_d;
__constant__ size_t N_max_bubble_d;
__constant__ long *rands; //array of random numbers
__device__ int d_has_overlap=0; //overlap indicator
__device__ int d_multi_tree=0;
extern __shared__ Bubble bs[];

//default global constant value in CPU
size_t N_max_bubble=10000; //max number of bubbles
size_t N_init=0;  //how many bubbles in the container at beginning
size_t N_max_step=1000; //steps of evolution
size_t N_start_step= 1; //the step that begins to output data
int N_steps_output=10; //skip how many steps for one output
float N_inject_rate=9; //how many bubbles injected per dt, which may be a float number or even zero
float InitR=1.0; //initial radius of the injected bubble, unit mm, must larger than R_min
float Width_container=40; //unit mm
float Height_container=100; //unit mm
float K_interval=0; //the ratio of interval between injected bubbles to InitR, which reflects the density
float Init_height=100; //the ratio of height that is filled with bubbles at initial time.
int Block_size=256;
int Rand_seed=0;
int Init_var=0; //0 means to set distribution for R while 1 means for R
int Output_var=0; //0 means to output R while 1 means V=4/3*PI*R^3
int InitDistFolder=1; //chose a folder which contains the distribution dll file.

//global variables, change frequently
size_t N_bubble=0; // current number of bubbles
size_t N_reusable=0;
long In=0;

/***************************************Functions in the GPU******************************************************/

__device__ float random(long seed, long* ptr) //random number generator
{

	long In=16807*(seed%127773)-2836*(seed/127773);
	if(In<0) In+=2147483647;
	*ptr=In;
	return (float)In/2147483647;
}

__global__ void moveBubbles(PBubble* ptr_bubbles, size_t N_bubble)
{
	size_t i = blockIdx.x * blockDim.x + threadIdx.x; //thread index
	if(i < N_bubble) //the thread index must be less than number of numbers
	{
		PBubble pb=ptr_bubbles[i];
		if(Hm_max>0) //move the bubbles in x-y plane
		{
			float x,y,theta,r;
			long seed=rands[i];//initialize the seed of random generator by the thread index
			do
			{
				theta=random(seed,&seed)*2*PI; // assume the horizontal moving vector is uniformly spread in a pie
				r=Hm_max;
				x=pb->x + r*cosf(theta);
				y=pb->y + r*sinf(theta);
			}while(x<0 || x> Width_container_d || y<0 || y> Width_container_d);
			pb->x = x;
			pb->y = y;
			rands[i]=seed;//store the seed back
		}	
		if(pb->z) pb->z +=K_velocity*pb->R*pb->R; //move up the bubble in z direction by K_velocity*R^2; 
		// set the invalid bubble's height to zero, so that it wouldn't affect the overlap detection
		if(pb->z > Height_container_d+ K_escape*pb->R) 
		{
			pb->z=0;
			d_has_overlap=1; //give a warning that there's bubble getting out of water
		}	
	}
}

__global__ void checkOverlap(PBubble* ptr_bubbles, size_t N_bubble)
{

	long base=blockIdx.x * blockDim.x;
	long i =base + threadIdx.x; //thread index
	bool first_overlap=false;

	if(i < N_bubble) //the thread index must be less than number of bubbles
	{	
		//load bubbles to shared memory
		PBubble sp=ptr_bubbles[i];
		bs[threadIdx.x].R=sp->R;
		bs[threadIdx.x].x=sp->x;
		bs[threadIdx.x].y=sp->y;
		bs[threadIdx.x].z=sp->z;
		__syncthreads(); //wait until all block threads finished loading

		float sR=bs[threadIdx.x].R;
		float sx=bs[threadIdx.x].x;
		float sy=bs[threadIdx.x].y;
		float sz=bs[threadIdx.x].z;
		float up_limit=sz+2*sR;
		float low_limit=sz-2*sR;

		////////////
		PBubble np;
		float nR,nx,ny,nz;
		//iterate to the left, which means larger z
		long j=i-1;
		while(j>=0)
		{

			if(j-base>=0 && j-base< blockDim.x) //j belong to this block, fetch data from shared memory, faster
			{
				nR=bs[j-base].R;
				nx=bs[j-base].x;
				ny=bs[j-base].y;
				nz=bs[j-base].z;
			}
			else //fetch from the global memory, slower
			{
				np=ptr_bubbles[j];
				nR=np->R;
				nx=np->x;
				ny=np->y;
				nz=np->z;
			}

			if(nz > up_limit) break; //exceed the detection range, quit
			if((sx-nx)*(sx-nx)+(sy-ny)*(sy-ny)+(sz-nz)*(sz-nz) < (sR+nR)*(sR+nR)) //they touched each other
			{
				if(!first_overlap)
				{
					sp->ip=j;
					atomicExch(&(ptr_bubbles[j]->iped),i);
					d_has_overlap=1;//atomicExch(&d_has_overlap,1);
					first_overlap=true;
				}
				else 
				{
					d_multi_tree=1;
					return; // finish detection for this bubble
				}	
			}
			--j;
		}
		//iterate to the right
		j=i+1;
		while(j<N_bubble)
		{
			if(j-base>=0 && j-base< blockDim.x) //j belong to this block
			{
				nR=bs[j-base].R;
				nx=bs[j-base].x;
				ny=bs[j-base].y;
				nz=bs[j-base].z;
			}
			else //fetch from the global memory
			{
				np=ptr_bubbles[j];
				nR=np->R;
				nx=np->x;
				ny=np->y;
				nz=np->z;
			}

			if(nz < low_limit) break; //exceed the detection range, quit
			if((sx-nx)*(sx-nx)+(sy-ny)*(sy-ny)+(sz-nz)*(sz-nz) < (sR+nR)*(sR+nR)) //they touched each other
			{	
				if(!first_overlap)
				{
					sp->ip=j;
					atomicExch(&(ptr_bubbles[j]->iped),i);
					d_has_overlap=1;//atomicExch(&d_has_overlap,1);
					first_overlap=true;
				}
				else 
				{
					d_multi_tree=1;
					return; // finish detection for this bubble
				}	
			}
			++j;
		}
	}
}

__global__ void multiTree2Chain(PBubble* ptr_bubbles, size_t N_bubble) //turn binary tree to chain
{
	size_t i = blockIdx.x * blockDim.x + threadIdx.x; //thread index
	if(i < N_bubble) //the thread index must be less than number of bubbles
	{
		size_t ipn=ptr_bubbles[i]->ip;
		if(ipn!=NULLD && ptr_bubbles[ipn]->iped!=i) 
		{
			ptr_bubbles[i]->ip=NULLD;	
			d_has_overlap=1; //which means we have to do the overlap detect again
		}
	}
}



__global__ void mergeBubbles(PBubble* ptr_bubbles, size_t N_bubble)
{
	size_t i = blockIdx.x * blockDim.x + threadIdx.x; //thread index
	if(i < N_bubble) //the thread index must be less than number of bubbles
	{

		PBubble pb=ptr_bubbles[i];
		size_t ip=pb->ip;
		if(ip!=NULLD) //it collides with others
		{
			//determine it's in a chain or a loop
			//if it's in  a loop, we have to find out the biggest index in this loop
			//or it's in a chain, just check if this bubble is pointed by another bubble(determine the head node)
			int status=0;

			for (size_t ipn=ptr_bubbles[ip]->ip,max_ip=i>ip?i:ip;;ipn=ptr_bubbles[ipn]->ip)
			{	

				if(ipn==NULLD) //it's a chain
				{
					if(pb->iped==NULLD) status=1; //it's also the head of the chain
					break;
				}
				else if(ipn==i) //it's a loop
				{
					if(max_ip==i) status=2; //it can be also viewed as the beginning of the loop
					break;
				}
				max_ip=max_ip>ipn ? max_ip : ipn;
			}

			if(status) //keep on merging along the chain or loop
			{
				float V=pb->R3;	
				float Vt=0;
				float sumx=V*pb->x;
				float sumy=V*pb->y;
				float sumz=V*pb->z;
				PBubble pbn;
				for (size_t ipn=ip; ipn!=NULLD && ipn!=i; ipn=pbn->ip)
				{
					pbn=ptr_bubbles[ipn];
					Vt=pbn->R3;
					sumx += Vt*pbn->x;
					sumy += Vt*pbn->y;
					sumz += Vt*pbn->z;
					V += Vt;
					pbn->z=0;
				}
				pb->x = sumx/V;
				pb->y = sumy/V;
				pb->z = sumz/V;
				pb->R3=V;
				pb->R=powf(V,1.0/3.0);	
			}
		}
	}
}

__global__ void resetMergeMark(PBubble* ptr_bubbles, size_t N_bubble) //based on one-to-one collision
{
	size_t i = blockIdx.x * blockDim.x + threadIdx.x; //thread index
	if(i < N_bubble) //the thread index must be less than number of bubbles
	{
		ptr_bubbles[i]->ip=NULLD;
		ptr_bubbles[i]->iped=NULLD;
	}
}

/***************************************Functions in the CPU******************************************************/

double random(long* nextIn=NULL); //Random number generator
double random(long* nextIn) //Random number generator
{
	//static long In=(long)time(NULL);

	In=16807*(In%127773)-2836*(In/127773);
	if(In<0) In+=2147483647;
	if(nextIn) *nextIn=In;
	return (double)In/2147483647;
}

void loadConfig()
{
	cudaError_t cudaStatus;
	cudaStatus = cudaSetDevice(0); // Choose which GPU to run on, change this on a multi-GPU system.
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
		getchar();
		exit(1);
	}

	//initialize the default constant data in the GPU
	float h_Hm_max=1.0f;//horizontal move very step, unit mm
	float h_K_velocity=0.01f; //the coefficient in the formula u=(2*g*rho/9/eta)*R^2
	float h_K_escape=0.67f; //determine the critical condition for the bubble to escape


	//load configuration from data file
	FILE *fp=fopen("configure.txt","r");
	if(fp)
	{
		using std::map;
		using std::string;
		char name[50];//,value[50]; 
		float temp;
		int ch=0;
		map<string,float> mp;
		while (fscanf(fp,"%s %f",name,&temp)!=-1)
		{
			while((ch=getc(fp))!='\n' && ch!=-1){};
			mp[name]=temp;
		}
		//global constant in the CPU 
		N_max_bubble=(size_t)mp["N_max_bubble="];
		N_init=(size_t)mp["N_init="];
		N_max_step=(size_t)mp["N_max_step="];
		N_start_step=(size_t)mp["N_start_step="];
		N_steps_output=(int)mp["N_steps_output="];
		N_inject_rate=mp["N_inject_rate="];
		InitR=mp["InitR="];
		Width_container=mp["Width_container="];
		Height_container=mp["Height_container="];
		K_interval=mp["K_interval="];
		Init_height=mp["Init_height="];
		Block_size=(int)mp["Block_size="];
		Rand_seed=(int)mp["Rand_seed="];
		Init_var=(int)mp["Init_var="];
		Output_var=(int)mp["Output_var="];
		InitDistFolder=(int)mp["InitDistFolder="];
		//global constant in the GPU 
		h_Hm_max=mp["Hm_max="];
		h_K_velocity=mp["K_velocity="];
		h_K_escape=mp["K_escape="];
		
		fclose(fp);
	}
	else
	{
		printf("WARNING:\nThe configure.txt file doesn't exist in current directory!\nSo the program will use default values, which may be not good.\n\nPress any key to continue.\n\n");
		getchar();
	}
	//copy the constant to the GPU
	cudaMemcpyToSymbol(Hm_max,&h_Hm_max,sizeof(float));	
	cudaMemcpyToSymbol(K_velocity,&h_K_velocity,sizeof(float));
	cudaMemcpyToSymbol(K_escape,&h_K_escape,sizeof(float));
	//copy the duplicated part
	cudaMemcpyToSymbol(Width_container_d,&Width_container,sizeof(float));
	cudaMemcpyToSymbol(Height_container_d,&Height_container,sizeof(float));
	cudaMemcpyToSymbol(NULLD,&N_max_bubble,sizeof(size_t));

	//initialize the random number array in the GPU;
	if(h_Hm_max>0)
	{
		long* rand_array=(long*)malloc(N_max_bubble*sizeof(long));
		for(size_t i=0; i<N_max_bubble; ++i) random(rand_array+N_max_bubble-1-i);  
		long* d_rands;
		cudaMalloc(&d_rands,N_max_bubble*sizeof(long));
		cudaMemcpy(d_rands,rand_array,N_max_bubble*sizeof(long),cudaMemcpyHostToDevice);
		cudaMemcpyToSymbol(rands,&d_rands,sizeof(long *));
		free(rand_array);
	}
	if(Rand_seed) In=Rand_seed;
	else In=(long)time(NULL);
}

void initContainer(thrust::device_vector<PBubble>& vec_bubbles) //initialize the container with given distribution
{
	double X;
	Bubble b;
	Bubble *pb;
	/*load the distribution generator from dll file*/
	SetCurrentDirectory("InitDistribution");
	char searchScope[10];
	sprintf(searchScope,"%d.*",InitDistFolder);
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(searchScope, &fd);
	bool found=false;
	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do{
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )//Ŀ¼
			{
				printf("The folder you designated is ");
				printf(fd.cFileName);
				printf("\n\n");
				SetCurrentDirectory(fd.cFileName);
				found=true;
				break;
			}
		}while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	if(!found)
	{
		printf("Cann't find the folder index %d you designated under 'InitDistribution'\n",InitDistFolder);
		printf("You must provide the right folder containing the right dll file to determine the initial distribution\n");
		getchar();
		exit(1);
	}

	HINSTANCE hdll=::LoadLibrary("Size.dll");
	if(!hdll)
	{
		printf("Error! the Size.dll is missing, which defines the initial distribution of the system.\n");
		getchar();
		exit(1);
	}
	typedef double (* DllFunc)(void); 
	DllFunc SizeofBubble = (DllFunc)GetProcAddress(hdll, "fnSize"); 
	if(!SizeofBubble)
	{
		printf("Error! Cannot get function 'fnSize' from Size.dll\n");
		getchar();
		exit(1);
	}
	/*end loading dll file*/
	
	for(size_t i=0;i<N_init;++i) 
	{
		X=SizeofBubble();
		//spread N_init bubbles in the container UNIFORMLY
		if(Init_var==0) // X is R
		{
			b.R=(float)X;
			b.R3=(float)(X*X*X);
		}
		else //X is V
		{
			b.R3=(float)(X/K_SPHERE);
			b.R=(float)pow(X/K_SPHERE,1.0/3.0);
		}

		b.x=(float)(b.R+random()*(Width_container-2*b.R));
		b.y=(float)(b.R+random()*(Width_container-2*b.R));
		b.z=(float)(b.R+random()*(Init_height-2*b.R));
		cudaMalloc(&pb,sizeof(Bubble));
		vec_bubbles[N_bubble]=pb;
		cudaMemcpy(pb,&b,sizeof(Bubble),cudaMemcpyHostToDevice);		
		++N_bubble;
	}
	if(SizeofBubble) ::FreeLibrary(hdll); 
	//SetCurrentDirectory("../..");
}

void addNewBubbles(thrust::device_vector<PBubble>& vec_bubbles)
{
	//add some new bubbles from the bottom of the container
	int N_inject=(int)N_inject_rate;
	float p=N_inject_rate-N_inject;//the decimal part
	if(p && random()<p) ++N_inject; //use random number to determine whether to add one more particle
	//randomly spread these injected bubbles
	size_t max_ix=(size_t)((Width_container/InitR-4)/(2+K_interval)+1); //max index of the possible positions of the injected bubble
	Bubble b;
	Bubble *pb;
	for(int n=0; n < N_inject; ++n) //push N_inject bubbles into the container
	{		
		//generate random integer pairs between [1, max_ix]
		size_t ix=(size_t)(random()*(max_ix-0.1)+1); 
		size_t iy=(size_t)(random()*(max_ix-0.1)+1);  	
		b.R = InitR;
		b.R3=InitR*InitR*InitR;
		b.x = (3+(2+K_interval)*(ix-1))*InitR;
		b.y = (3+(2+K_interval)*(iy-1))*InitR;
		b.z = InitR;

		if(N_reusable)	--N_reusable; //try to reuse the memory in GPU, which will save time of memory allocation
		else
		{
			//allocate memory for this bubble
			cudaMalloc(&pb,sizeof(Bubble));
			vec_bubbles[N_bubble]=pb;		
		}
		cudaMemcpy(vec_bubbles[N_bubble],&b,sizeof(Bubble),cudaMemcpyHostToDevice);		
		++N_bubble;
	}
}

void sortBubbles(thrust::device_vector<PBubble>& vec_bubbles) //sort the bubbles by their height, and retrieve memories
{
	thrust::sort(vec_bubbles.begin(),vec_bubbles.begin()+N_bubble,compare_height()); 
	cudaStreamSynchronize(0);
	//try to retrieve some allocated data
	size_t index=N_bubble;
	PBubble pb=NULL;
	float z;
	do 
	{
		--index;
		if(index==0)
		{
			printf("\n\nWARNING:\nNo colliable bubble left! The simulation terminated earlier than expected!\n" \
				"Please adjust the parameters and run again.\n\nPress any key to continue...\n");
			getchar();
			exit(1);
		}
		pb=vec_bubbles[index];
		cudaMemcpy(&z,&(pb->z),sizeof(float),cudaMemcpyDeviceToHost);
	} while (z==0);
	++index;
	N_reusable+=N_bubble-index;
	N_bubble=index;
}

int overlapDetect(thrust::device_vector<PBubble>& vec_bubbles) //detect and mark collisions in pairs
{
	sortBubbles(vec_bubbles); //sort before we can detect overlap
	resetMergeMark<<<grid_size, Block_size>>>(VEC_D,N_bubble); //must reset the links first
	checkOverlap<<<grid_size, Block_size, Block_size*sizeof(Bubble)>>>(VEC_D,N_bubble);
	cudaStreamSynchronize(0); //wait until all threads end
	int h_has_overlap=0,h_multi_tree=0;
	int reset=0;
	cudaMemcpyFromSymbol(&h_has_overlap,d_has_overlap,sizeof(int));
	cudaMemcpyToSymbol(d_has_overlap,&reset,sizeof(int)); //reset the indication in the GPU to zero

	if(h_has_overlap) //we need to find out what kind of overlap
	{
		multiTree2Chain<<<grid_size, Block_size>>>(VEC_D,N_bubble);
		cudaStreamSynchronize(0); //wait for all threads end
		cudaMemcpyFromSymbol(&h_has_overlap,d_has_overlap,sizeof(int));
		cudaMemcpyToSymbol(d_has_overlap,&reset,sizeof(int)); //reset the indication in the GPU to zero

		cudaMemcpyFromSymbol(&h_multi_tree,d_multi_tree,sizeof(int));
		cudaMemcpyToSymbol(d_multi_tree,&reset,sizeof(int)); //reset the indication in the GPU to zero

		if(h_has_overlap||h_multi_tree) return 2; //there are multi trees that need repeat detection
		return 1; 
	}
	return 0; //there's no ovelap
}

void moveBubbles(thrust::device_vector<PBubble>& vec_bubbles, size_t& N_step)
{
	moveBubbles<<<grid_size, Block_size>>>(VEC_D,N_bubble); //move bubbles
	cudaStreamSynchronize(0); //wait until all threads end
	int h_has_overlap=0;
	int reset=0;
	cudaMemcpyFromSymbol(&h_has_overlap,d_has_overlap,sizeof(int));
	cudaMemcpyToSymbol(d_has_overlap,&reset,sizeof(int)); //reset the indicator in the GPU to zero
	if(h_has_overlap) //here it means at least one bubble starts getting out of water
	{
		printf("\nCaution!\nAt least one bubble starts getting out of water at step=%d, please adjust the N_max_step or K_velocity, then restart the simulation.\n",N_step);
	}
}

int main()
{
	loadConfig(); //load some constants
	clock_t pre_time=clock(),now_time;
	thrust::device_vector<PBubble> vec_bubbles(N_max_bubble, NULL); //vector which contains pointers to the real data
	int overlap_status=0;
	initContainer(vec_bubbles);
	FILE* fp=fopen("status.txt","w"); //prepare to dump the runtime information
	if(fp==NULL)
	{
		printf("Cannot write file status.txt!");
		getchar();
		exit(1);
	}
	fprintf(fp,"width= %.4f\t\theight= %.4f\t\tnumber_snapshot= %d\t\tdata_type= %d\n",Width_container,Height_container,(N_max_step-N_start_step)/N_steps_output+1,Output_var);
	fprintf(fp,"*********************************************************************************************************\n\n");
	
	Bubble b;
	if(N_start_step==0) //we need to output the initial state
	{	
		fprintf(fp,"step= %d\t\tbubble_number= %d\n",0,N_bubble);
		for(size_t i=0; i<N_bubble; ++i)
		{
			cudaMemcpy(&b,vec_bubbles[i],sizeof(Bubble),cudaMemcpyDeviceToHost); //copy the data from the GPU to CPU
			if(Output_var==0) fprintf(fp,"% .4f\t\t\t% .4f\t\t\t% .4f\t\t\t% .4f\n", b.x, b.y, b.z, b.R);
			else fprintf(fp,"% .4f\t\t\t% .4f\t\t\t% .4f\t\t\t% .4f\n", b.x, b.y, b.z, K_SPHERE*b.R3);
		}			
		fprintf(fp,"*********************************************************************************************************\n\n\n");
		//tick the time
		now_time=clock();
		printf("the %dth step has finished, %d bubbles costing %f secs.\n",0,N_bubble,(now_time-pre_time)/(float)CLOCKS_PER_SEC);
		pre_time=now_time;
	}
	for (size_t step=1; step <= N_max_step; ++step) //repeat the simulation until N_max_step
	{
		if(N_inject_rate>0) addNewBubbles(vec_bubbles); //pump in some new bubbles
		moveBubbles(vec_bubbles,step);
		do  //the Overlap Detection depends on the ordered sequence
		{
			overlap_status=overlapDetect(vec_bubbles);
			if (overlap_status)
			{
				//deal with the merge
				mergeBubbles<<<grid_size, Block_size>>>(VEC_D,N_bubble); //merge bubbles, and have eliminated some bubbles
				cudaStreamSynchronize(0); //wait until all threads end
			}	
		}while (overlap_status==2);  

		//output information
		if((step-N_start_step)%N_steps_output==0 && step>=N_start_step)
		{
			//dump the status of whole container			
			fprintf(fp,"step= %d\t\tbubble_number= %d\n",step,N_bubble);
			for(size_t i=0; i<N_bubble; ++i)
			{
				cudaMemcpy(&b,vec_bubbles[i],sizeof(Bubble),cudaMemcpyDeviceToHost); //copy the data from the GPU
				if(Output_var==0) fprintf(fp,"% .4f\t\t\t% .4f\t\t\t% .4f\t\t\t% .4f\n", b.x, b.y, b.z, b.R);
				else fprintf(fp,"% .4f\t\t\t% .4f\t\t\t% .4f\t\t\t% .4f\n", b.x, b.y, b.z, K_SPHERE*b.R3);
			}			
			fprintf(fp,"*********************************************************************************************************\n\n\n");
			//tick the time
			now_time=clock();
			printf("the %dth step has finished, %d bubbles costing %f secs.\n",step,N_bubble,(now_time-pre_time)/(float)CLOCKS_PER_SEC);
			pre_time=now_time;
		}
	}

	fclose(fp);
	printf("\nTotal Elapsed Time:%f secs.\n",clock()/(float)CLOCKS_PER_SEC);
	printf("Press any key to continue!\n");
	getchar();
	//SetCurrentDirectory("../..");
	ShellExecute(NULL,"open","VisualBubbles.exe",NULL,NULL,SW_SHOWNORMAL); //this is windows platform specific
	return 0;
}