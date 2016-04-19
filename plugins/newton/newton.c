//
//  General-purpose Photovoltaic Device Model gpvdm.com- a drift diffusion
//  base/Shockley-Read-Hall model for 1st, 2nd and 3rd generation solarcells.
// 
//  Copyright (C) 2012 Roderick C. I. MacKenzie
//
//	roderick.mackenzie@nottingham.ac.uk
//	www.roderickmackenzie.eu
//	Room B86 Coates, University Park, Nottingham, NG7 2RD, UK
//
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.



#include <string.h>
#include <log.h>
#include <const.h>
#include "newton.h"
#include <dll_export.h>
#include <util.h>
#include <exp.h>
#include <advmath.h>
#include <dump.h>
#include <cal_path.h>
#include <dos.h>
#include <sim.h>
#include <solver_interface.h>


static gdouble Jnl=0.0;
static gdouble Jnr=0.0;
static gdouble Jpl=0.0;
static gdouble Jpr=0.0;

static gdouble Dnl=0.0;
static gdouble Dnc=0.0;
static gdouble Dnr=0.0;
static gdouble Dpl=0.0;
static gdouble Dpc=0.0;
static gdouble Dpr=0.0;

static gdouble nl=0.0;
static gdouble nc=0.0;
static gdouble nr=0.0;

static gdouble pl=0.0;
static gdouble pc=0.0;
static gdouble pr=0.0;

static gdouble xil=0.0;
static gdouble xir=0.0;
static gdouble xipl=0.0;
static gdouble xipr=0.0;

static gdouble dJpdxipl=0.0;
static gdouble dJpdxipc=0.0;
static gdouble dJpdxipr=0.0;

static gdouble dnl=0.0;
static gdouble dnc=0.0;
static gdouble dnr=0.0;

static gdouble dpl=0.0;
static gdouble dpc=0.0;
static gdouble dpr=0.0;

static gdouble munl=0.0;
static gdouble munc=0.0;
static gdouble munr=0.0;

static gdouble mupl=0.0;
static gdouble mupc=0.0;
static gdouble mupr=0.0;


static gdouble wnl=0.0;
static gdouble wnc=0.0;
static gdouble wnr=0.0;

static gdouble wpl=0.0;
static gdouble wpc=0.0;
static gdouble wpr=0.0;

static gdouble dJdxil=0.0;
static gdouble dJdxic=0.0;
static gdouble dJdxir=0.0;

static gdouble dJdphil=0.0;
static gdouble dJdphic=0.0;
static gdouble dJdphir=0.0;

static gdouble dJpdphil=0.0;
static gdouble dJpdphic=0.0;
static gdouble dJpdphir=0.0;


static gdouble dphidxic=0.0;
static gdouble dphidxipc=0.0;


void update_solver_vars(struct simulation *sim,struct device *in,int clamp)
{
int i;
int band=0;

gdouble clamp_temp=300.0;

gdouble update=0.0;
	for (i=0;i<in->ymeshpoints;i++)
	{

		update=(gdouble)in->b[i];
		if ((in->interfaceleft==TRUE)&&(i==0))
		{
		}else
		if ((in->interfaceright==TRUE)&&(i==in->ymeshpoints-1))
		{
		}else
		{
			if (clamp==TRUE)
			{
				in->phi[i]+=update/(1.0+gfabs(update/in->electrical_clamp/(clamp_temp*kb/Q)));
			}else
			{
				in->phi[i]+=update;

			}
		}


		update=(gdouble)(in->b[in->ymeshpoints*(1)+i]);
		if (clamp==TRUE)
		{
			in->x[i]+=update/(1.0+gfabs(update/in->electrical_clamp/(clamp_temp*kb/Q)));
		}else
		{
			in->x[i]+=update;
		}


		update=(gdouble)(in->b[in->ymeshpoints*(1+1)+i]);
		if (clamp==TRUE)
		{
			in->xp[i]+=update/(1.0+gfabs(update/in->electrical_clamp/(clamp_temp*kb/Q)));
		}else
		{
			in->xp[i]+=update;

		}


		if (in->ntrapnewton==TRUE)
		{
			for (band=0;band<in->srh_bands;band++)
			{
				update=(gdouble)(in->b[in->ymeshpoints*(1+1+1+band)+i]);
				if (clamp==TRUE)
				{
					in->xt[i][band]+=update/(1.0+gfabs(update/in->electrical_clamp/(clamp_temp*kb/Q)));

				}else
				{
					in->xt[i][band]+=update;
				}
			}
		}

		if (in->ptrapnewton==TRUE)
		{
			for (band=0;band<in->srh_bands;band++)
			{
				update=(gdouble)(in->b[in->ymeshpoints*(1+1+1+in->srh_bands+band)+i]);
				if (clamp==TRUE)
				{
					in->xpt[i][band]+=update/(1.0+gfabs(update/in->electrical_clamp/(clamp_temp*kb/Q)));
				}else
				{
					in->xpt[i][band]+=update;

				}
			}
		}


		}


update_arrays(sim,in);

}

void fill_matrix(struct simulation *sim,struct device *in)
{
//gdouble offset= -0.5;
int band=0;
update_arrays(sim,in);
//FILE *file_j =fopen("myj.dat","w");
//getchar();
gdouble phil;
gdouble phic;
gdouble phir;
gdouble yl;
gdouble yc;
gdouble yr;
gdouble dyl;
gdouble dyr;
gdouble ddh=0.0;
//gdouble dh;
int pos=0;

gdouble Ecl=0.0;
gdouble Ecr=0.0;
gdouble Ecc=0.0;
gdouble Evl=0.0;
gdouble Evr=0.0;
gdouble Evc=0.0;

gdouble Tel=0.0;
//gdouble Tec=0.0;
gdouble Ter=0.0;

gdouble Thl=0.0;
//gdouble Thc=0.0;
gdouble Thr=0.0;

gdouble xnr;
gdouble tnr;
gdouble xnl;
gdouble tnl;

gdouble xpr;
gdouble tpr;
gdouble xpl;
gdouble tpl;


//gdouble exl;
//gdouble exr;
//gdouble exc;
//gdouble Dexl;
//gdouble Dexc;
//gdouble Dexr;
//gdouble R;

gdouble epr;
gdouble epc;
gdouble epl;

//gdouble G;
gdouble Gn;
gdouble Gp;
int i;
gdouble dJdxipc=0.0;
gdouble dJpdxic=0.0;

gdouble e0=0.0;
gdouble e1=0.0;

gdouble dphil=0.0;
gdouble dphic=0.0;
gdouble dphir=0.0;
gdouble deriv=0.0;

gdouble nlast=0.0;
gdouble plast=0.0;
gdouble dt=0.0;

//gdouble kll=0.0;
//gdouble klc=0.0;
//gdouble klr=0.0;

//gdouble kl0=0.0;
//gdouble kl1=0.0;

gdouble one=0.0;
gdouble one0_l=0.0;
gdouble one0_r=0.0;



gdouble Rtrapn=0.0;
gdouble Rtrapp=0.0;

gdouble dJdphil_leftl=0.0;
gdouble dJdphil_leftc=0.0;
gdouble dJpdphil_leftl=0.0;
gdouble dJpdphil_leftc=0.0;
gdouble dphil_left=0.0;
gdouble dJdxil_leftc=0.0;
gdouble dJpdxipl_leftc=0.0;

gdouble dJdxic_rightc=0.0;
gdouble dJpdxipc_rightc=0.0;
gdouble dJpdphi_rightc=0.0;
gdouble dJdphi_rightc=0.0;

gdouble Bfree=0.0;
gdouble nceq=0.0;
gdouble pceq=0.0;
gdouble Rfree=0.0;

gdouble nc0_l=0.0;
//gdouble dnc0_l=0.0;
//gdouble pc0_l=0.0;
//gdouble dpc0_l=0.0;

gdouble nc0_r=0.0;
//gdouble dnc0_r=0.0;
//gdouble pc0_r=0.0;
//gdouble dpc0_r=0.0;

gdouble dJnldxil_l=0.0;
gdouble dJnldxil_c=0.0;
gdouble dJnrdxir_c=0.0;
gdouble dJnrdxir_r=0.0;
gdouble dJpldxipl_l=0.0;
gdouble dJpldxipl_c=0.0;
gdouble dJprdxipr_c=0.0;
gdouble dJprdxipr_r=0.0;

gdouble i0=0.0;
gdouble didv=0.0;  //not a function
gdouble diphic=0.0; //could be a function
gdouble didxic=0.0;
gdouble didxipc=0.0;
gdouble didphir=0.0;
gdouble didxir=0.0;
gdouble didxipr=0.0;
gdouble Nad=0.0;

//gdouble dylh_left=0.0;
//gdouble dyrh_left=0.0;
//gdouble dncdphic=0.0;
//gdouble dpcdphic=0.0;

if (in->kl_in_newton==FALSE)
{
	if (in->interfaceleft==TRUE)
	{
			in->phi[0]=in->Vapplied;
	}
}

if (in->interfaceright==TRUE)
{
		in->phi[in->ymeshpoints-1]=in->Vr;
}

		pos=0;
		for (i=0;i<in->ymeshpoints;i++)
		{
			if (i==0)
			{
//				exl=0.0;
//				Dexl=in->Dex[0];

					phil=(in->Vapplied);
					//printf("setr=%Lf %Lf\n",phil,in->Vapplied);

				yl=in->ymesh[0]-(in->ymesh[1]-in->ymesh[0]);
//				Tll=in->Tll;
				Tel=in->Tll;
				Thl=in->Tll;




				Ecl= -in->Xi[0]-phil;
				Evl= -in->Xi[0]-phil-in->Eg[0];
				epl=in->epsilonr[0]*epsilon0;


				xnl=in->Fi[0];
				tnl=in->Xi[0];
				one=xnl+tnl;

				nl=get_n_den(in,one,Tel,in->imat[i]);
				dnl=get_dn_den(in,one,Tel,in->imat[i]);
				wnl=get_n_w(in,one,Tel,in->imat[i]);
				
				munl=in->mun[0];


				xpl= -in->Fi[0];
				tpl=(in->Xi[0]+in->Eg[0]);
				one=xpl-tpl;

				pl=get_p_den(in,one,Thl,in->imat[i]);
				dpl=get_dp_den(in,one,Thl,in->imat[i]);
				wpl=get_p_w(in,one,Thl,in->imat[i]);
				

				mupl=in->mup[0];
			
				//printf("left n= %Le p= %Le \n",nl,pl);


//				kll=in->kl[i];

			}else
			{
//				Dexl=in->Dex[i-1];
//				exl=in->ex[i-1];				
				phil=in->phi[i-1];
				yl=in->ymesh[i-1];
//				Tll=in->Tl[i-1];
				Tel=in->Te[i-1];
				Thl=in->Th[i-1];
				Ecl=in->Ec[i-1];
				Evl=in->Ev[i-1];


				nl=in->n[i-1];
				dnl=in->dn[i-1];


				wnl=in->wn[i-1];
				wpl=in->wp[i-1];

				pl=in->p[i-1];
				dpl=in->dp[i-1];
				munl=in->mun[i-1];
				mupl=in->mup[i-1];


				epl=in->epsilonr[i-1]*epsilon0;


//				kll=in->kl[i-1];
			}

			Ecc=(-in->Xi[i]-in->phi[i]);
			Evc=(-in->Xi[i]-in->phi[i]-in->Eg[i]);

			if (i==(in->ymeshpoints-1))
			{

//				Dexr=in->Dex[i];
//				exr=0.0;			
				//phir=in->Vr;

					phir=in->Vr;

				yr=in->ymesh[i]+(in->ymesh[i]-in->ymesh[i-1]);
//				Tlr=in->Tlr;
				Ter=in->Tlr;
				Thr=in->Tlr;


				Ecr= -in->Xi[i]-phir;
				Evr= -in->Xi[i]-phir-in->Eg[i];


				xnr=(in->Vr+in->Fi[i]);
				tnr=(in->Xi[i]);

				one=xnr+tnr;

				nr=get_n_den(in,one,Ter,in->imat[i]);
				dnr=get_dn_den(in,one,Ter,in->imat[i]);
				wnr=get_n_w(in,one,Ter,in->imat[i]);



				xpr= -(in->Vr+in->Fi[i]);
				tpr=(in->Xi[i]+in->Eg[i]);

				one=xpr-tpr;

				pr=get_p_den(in,one,Thr,in->imat[i]);
				dpr=get_dp_den(in,one,Thr,in->imat[i]);
				wpr=get_p_w(in,one,Thr,in->imat[i]);

				munr=in->mun[i];
				mupr=in->mup[i];

				//printf("right n= %Le p= %Le \n",nr,pr);

				epr=in->epsilonr[i]*epsilon0;
//				klr=in->kl[i];

			}else
			{

//				Dexr=in->Dex[i+1];
//				exr=in->ex[i+1];				
				phir=in->phi[i+1];
				yr=in->ymesh[i+1];
//				Tlr=in->Tl[i+1];
				Ter=in->Te[i+1];
				Thr=in->Th[i+1];

				Ecr=in->Ec[i+1];
				Evr=in->Ev[i+1];


				nr=in->n[i+1];
				dnr=in->dn[i+1];

				wnr=in->wn[i+1];
				wpr=in->wp[i+1];

				pr=in->p[i+1];
				dpr=in->dp[i+1];
				munr=in->mun[i+1];
				mupr=in->mup[i+1];

				epr=in->epsilonr[i+1]*epsilon0;
//				klr=in->kl[i+1];

			}

			dJdxipc=0.0;
			dJpdxic=0.0;

			epc=in->epsilonr[i]*epsilon0;


//			exc=in->ex[i];
//			Dexc=in->Dex[i];
			yc=in->ymesh[i];
			dyl=yc-yl;
			dyr=yr-yc;
			ddh=(dyl+dyr)/2.0;
			gdouble dylh=dyl/2.0;
			gdouble dyrh=dyr/2.0;

//			dh=(dyl+dyr);
			phic=in->phi[i];
//			Tlc=in->Tl[i];
//			Tec=in->Te[i];
//			Thc=in->Th[i];

				munc=in->mun[i];
				mupc=in->mup[i];

				wnc=in->wn[i];
				wpc=in->wp[i];

				Dnl=munl*(2.0/3.0)*wnl/Q;
				Dpl=mupl*(2.0/3.0)*wpl/Q;

				Dnc=munc*(2.0/3.0)*wnc/Q;
				Dpc=mupc*(2.0/3.0)*wpc/Q;
				in->Dn[i]=Dnc;
				in->Dp[i]=Dnc;


				Dnr=munr*(2.0/3.0)*wnr/Q;
				Dpr=mupr*(2.0/3.0)*wpr/Q;


				Dnl=(Dnl+Dnc)/2.0;
				Dnr=(Dnr+Dnc)/2.0;

				Dpl=(Dpl+Dpc)/2.0;
				Dpr=(Dpr+Dpc)/2.0;

				munl=(munl+munc)/2.0;
				munr=(munr+munc)/2.0;

				mupl=(mupl+mupc)/2.0;
				mupr=(mupr+mupc)/2.0;

	

				nc=in->n[i];
				pc=in->p[i];

				dnc=in->dn[i];
				dpc=in->dp[i];
//				dncdphic=in->dndphi[i];
//				dpcdphic=in->dpdphi[i];

				Bfree=in->B[i];
				Nad=in->Nad[i];
				nceq=in->nfequlib[i];
				pceq=in->pfequlib[i];
				Rfree=Bfree*(nc*pc-nceq*pceq);
				in->Rfree[i]=Rfree;
				//if (in->ymeshpoints*(1)+i==31) printf("Rod -- %d %le %le %le \n",in->ymeshpoints*(1)+i,Rfree,nceq,pceq);

//			klc=in->kl[i];
			nlast=in->nlast[i];
			plast=in->plast[i];

			for (band=0;band<in->srh_bands;band++)
			{
				in->newton_ntlast[band]=in->ntlast[i][band];
				in->newton_ptlast[band]=in->ptlast[i][band];
			}

			dt=in->dt;

//	R=in->R[i];
	Gn=in->Gn[i];
	Gp=in->Gp[i];
	e0=(epl+epc)/2.0;
	e1=(epc+epr)/2.0;

//	kl0=(klc+kll)/2.0;
//	kl1=(klr+klc)/2.0;

	dphil= -e0/dyl/ddh;
	dphic= e0/dyl/ddh+e1/dyr/ddh;
	dphir= -e1/dyr/ddh;

	gdouble dphil_d=dphil;
	gdouble dphic_d=dphic;
	gdouble dphir_d=dphir;




	//if (i==in->ymeshpoints-1)
	//{
	//printf("%le\n",phir);
//
	//}
	deriv=phil*dphil+phic*dphic+phir*dphir;
	//if (in->Vapplied>0.1)
	//{
	//	printf("%Le %Le %Le %Le %Le\n",phil,phic,phir,nc,pc);
	//	getchar();
	//}
	dphidxic=Q*(dnc);
	dphidxipc= -Q*(dpc);

	if (((in->interfaceleft==TRUE)&&(i==0))||((in->interfaceright==TRUE)&&(i==in->ymeshpoints-1)))
	{
		dphidxic=0.0;
		dphidxipc=0.0;
	}

	if (in->ntrapnewton==TRUE)
	{
		for (band=0;band<in->srh_bands;band++)
		{
			in->newton_dphidntrap[band]=Q*(in->dnt[i][band]);
			if ((in->interfaceleft==TRUE)&&(i==0)) in->newton_dphidntrap[band]=0.0;
			if ((in->interfaceright==TRUE)&&(i==in->ymeshpoints-1)) in->newton_dphidntrap[band]=0.0;
		}
	}

	if (in->ptrapnewton==TRUE)
	{
		for (band=0;band<in->srh_bands;band++)
		{
			in->newton_dphidptrap[band]= -Q*(in->dpt[i][band]);
			if ((in->interfaceleft==TRUE)&&(i==0)) in->newton_dphidptrap[band]=0.0;
			if ((in->interfaceright==TRUE)&&(i==in->ymeshpoints-1)) in->newton_dphidptrap[band]=0.0;

			//dphidxipc+= -Q*(in->dpt[i]);
		}
	}




//	G=in->G[i];



			xil=Q*2.0*(3.0/2.0)*(Ecc-Ecl)/((wnc+wnl));
			xir=Q*2.0*(3.0/2.0)*(Ecr-Ecc)/((wnr+wnc));

			//gdouble dxil= -Q*2.0*(3.0/2.0)*(Ecc-Ecl)/pow((wnc+wnl),2.0);
			//gdouble dxir= -Q*2.0*(3.0/2.0)*(Ecr-Ecc)/pow((wnr+wnc),2.0);

			xipl=Q*2.0*(3.0/2.0)*(Evc-Evl)/(wpc+wpl);
			xipr=Q*2.0*(3.0/2.0)*(Evr-Evc)/(wpr+wpc);

			dJdxil=0.0;
			dJdxic=0.0;
			dJdxir=0.0;

			dJpdxipl=0.0;
			dJpdxipc=0.0;
			dJpdxipr=0.0;


			dJdphil=0.0;
			dJdphic=0.0;
			dJdphir=0.0;


			dJpdphil=0.0;
			dJpdphic=0.0;
			dJpdphir=0.0;

	
			Jnl=(Dnl/dyl)*(B(-xil)*nc-B(xil)*nl);
			dJnldxil_l= -(Dnl/dyl)*(B(xil)*dnl);
			dJnldxil_c=(Dnl/dyl)*B(-xil)*dnc;

			gdouble dJnldphi_l= -(munl/dyl)*(dB(-xil)*nc+dB(xil)*nl);
			gdouble dJnldphi_c=(munl/dyl)*(dB(-xil)*nc+dB(xil)*nl);

			Jnr=(Dnr/dyr)*(B(-xir)*nr-B(xir)*nc);
			dJnrdxir_c= -(Dnr/dyr)*(B(xir)*dnc);
			dJnrdxir_r=(Dnr/dyr)*(B(-xir)*dnr);

			gdouble dJnrdphi_c=(munr/dyr)*(-dB(-xir)*nr-dB(xir)*nc);
			gdouble dJnrdphi_r=(munr/dyr)*(dB(-xir)*nr+dB(xir)*nc);

			Jpl=(Dpl/dyl)*(B(-xipl)*pl-B(xipl)*pc);
			dJpldxipl_l=(Dpl/dyl)*(B(-xipl)*dpl);
			dJpldxipl_c= -(Dpl/dyl)*B(xipl)*dpc;

			gdouble dJpldphi_l= -((mupl)/dyl)*(dB(-xipl)*pl+dB(xipl)*pc);
			gdouble dJpldphi_c=((mupl)/dyl)*(dB(-xipl)*pl+dB(xipl)*pc);

			Jpr=(Dpr/dyr)*(B(-xipr)*pc-B(xipr)*pr);
			dJprdxipr_c=(Dpr/dyr)*(B(-xipr)*dpc);
			dJprdxipr_r= -(Dpr/dyr)*(B(xipr)*dpr);

			gdouble dJprdphi_c= -(mupr/dyr)*(dB(-xipr)*pc+dB(xipr)*pr);
			gdouble dJprdphi_r=(mupr/dyr)*(dB(-xipr)*pc+dB(xipr)*pr);


			if (i==0)
			{
				//printf("%le %le %le %le\n",in->vl*(nc-nc0_l),Jnl,-in->vl*(pc-pc0_l),Jpl);
				//getchar();
				in->Jnleft=Jnl;
				in->Jpleft=Jpl;
			}

			if (i==in->ymeshpoints-1)
			{
				//printf("%le %le %le %le\n",in->vr*(nc-nc0_r),Jnr,in->vr*(pc-pc0_r),Jpr);
				in->Jnright=Jnr;
				in->Jpright=Jpr;
			}

			//printf("----------\n");
			//printf("%le %le\n",Jnl,Jpl);
			//printf("%le %le\n",Jnr,Jpr);

			in->Jn[i]=Q*(Jnl+Jnr)/2.0;
			in->Jp[i]=Q*(Jpl+Jpr)/2.0;

			dJdxil+= -dJnldxil_l/(dylh+dyrh);
			dJdxic+=(-dJnldxil_c+dJnrdxir_c)/(dylh+dyrh);
			dJdxir+=dJnrdxir_r/(dylh+dyrh);

			dJpdxipl+= -dJpldxipl_l/(dylh+dyrh);
			dJpdxipc+=(-dJpldxipl_c+dJprdxipr_c)/(dylh+dyrh);
			dJpdxipr+=dJprdxipr_r/(dylh+dyrh);


			dJdphil+= -dJnldphi_l/(dylh+dyrh);
			dJdphic+=(-dJnldphi_c+dJnrdphi_c)/(dylh+dyrh);
			dJdphir+=dJnrdphi_r/(dylh+dyrh);


			dJpdphil+= -dJpldphi_l/(dylh+dyrh);
			dJpdphic+=(-dJpldphi_c+dJprdphi_c)/(dylh+dyrh);
			dJpdphir+=dJprdphi_r/(dylh+dyrh);

			if (Bfree!=0.0)
			{
				dJdxic+= -Bfree*(dnc*pc);
				dJdxipc+= -Bfree*(nc*dpc);

				dJpdxipc+=Bfree*(nc*dpc);
				dJpdxic+=Bfree*(dnc*pc);

				if ((in->interfaceleft==TRUE)&&(i==0))
				{
				}else
				if ((in->interfaceright==TRUE)&&(i==in->ymeshpoints-1))
				{
				}else
				{
					dJdphic+= -Bfree*(dnc*pc);
					dJpdphic+=Bfree*(nc*dpc);
				}

			}

			if (i==0)
			{

				dJdphil_leftl=dJnldphi_l;
				dJdphil_leftc=dJnldphi_c;
				dJpdphil_leftl=dJpldphi_l;
				dJpdphil_leftc=dJpldphi_c;
				//printf("%le %le\n",dpc,dnc);
				dphil_left= -e0/dyl/ddh;
				dJdxil_leftc=dJnldxil_c;
				dJpdxipl_leftc=dJpldxipl_c;
				//dylh_left=dylh;
				//dyrh_left=dyrh;

			}

			if (i==in->ymeshpoints-1)
			{
				dJdxic_rightc=dJnrdxir_c;
				dJpdxipc_rightc=dJprdxipr_c;
				dJdphi_rightc= -dJnrdphi_r;
				dJpdphi_rightc=dJprdphi_c;
			}

			if (in->go_time==TRUE)
			{
				dJdxic+= -dnc/dt;
			}

			if (in->go_time==TRUE)
			{
				dJpdxipc+=dpc/dt;
			}



			Rtrapn=0.0;
			Rtrapp=0.0;



			in->nrelax[i]=0.0;
			in->ntrap_to_p[i]=0.0;
			in->prelax[i]=0.0;
			in->ptrap_to_n[i]=0.0;


			if (in->ntrapnewton==TRUE)
			{
				for (band=0;band<in->srh_bands;band++)
				{
					in->newton_dJdtrapn[band]=0.0;
					in->newton_dJpdtrapn[band]=0.0;
					in->newton_dntrap[band]=nc*in->srh_n_r1[i][band]-in->srh_n_r2[i][band]-pc*in->srh_n_r3[i][band]+in->srh_n_r4[i][band];
					in->newton_dntrapdntrap[band]=nc*in->dsrh_n_r1[i][band]-in->dsrh_n_r2[i][band]-pc*in->dsrh_n_r3[i][band]+in->dsrh_n_r4[i][band];
					in->newton_dntrapdn[band]=dnc*in->srh_n_r1[i][band];
					in->newton_dntrapdp[band]= -dpc*in->srh_n_r3[i][band];
					Rtrapn+=nc*in->srh_n_r1[i][band]-in->srh_n_r2[i][band];
					dJdxic-=dnc*in->srh_n_r1[i][band];
					in->newton_dJdtrapn[band]-=nc*in->dsrh_n_r1[i][band]-in->dsrh_n_r2[i][band];
					Rtrapp+= -(-pc*in->srh_n_r3[i][band]+in->srh_n_r4[i][band]);
					dJpdxipc+= -(-dpc*in->srh_n_r3[i][band]);
					in->newton_dJpdtrapn[band]= -(-pc*in->dsrh_n_r3[i][band]+in->dsrh_n_r4[i][band]);

					if (in->go_time==TRUE)
					{
						in->newton_dntrap[band]+= -(in->nt[i][band]-in->newton_ntlast[band])/dt;
						in->newton_dntrapdntrap[band]+= -(in->dnt[i][band])/dt;
					}
					
					in->nrelax[i]+=nc*in->srh_n_r1[i][band]-in->srh_n_r2[i][band];
					in->ntrap_to_p[i]+= -(-pc*in->srh_n_r3[i][band]+in->srh_n_r4[i][band]);

					in->nt_r1[i][band]=nc*in->srh_n_r1[i][band];
					in->nt_r2[i][band]=in->srh_n_r2[i][band];
					in->nt_r3[i][band]=pc*in->srh_n_r3[i][band];
					in->nt_r4[i][band]=in->srh_n_r4[i][band];

				}
			}

			//band=0;
			//printf("heren %le %le %le %le\n",in->Vapplied,nc*in->srh_n_r1[i][band]-in->srh_n_r2[i][band]-pc*in->srh_n_r3[i][band]+in->srh_n_r4[i][band],nc*in->srh_n_r1[i][band]-in->srh_n_r2[i][band],-pc*in->srh_n_r3[i][band]+in->srh_n_r4[i][band]);

			if (in->ptrapnewton==TRUE)
			{

				for (band=0;band<in->srh_bands;band++)
				{
					//dJdtrapn[band]=0.0;
					in->newton_dJpdtrapp[band]=0.0;
					in->newton_dJdtrapp[band]=0.0;
					in->newton_dptrap[band]=pc*in->srh_p_r1[i][band]-in->srh_p_r2[i][band]-nc*in->srh_p_r3[i][band]+in->srh_p_r4[i][band];
					in->newton_dptrapdptrap[band]=pc*in->dsrh_p_r1[i][band]-in->dsrh_p_r2[i][band]-nc*in->dsrh_p_r3[i][band]+in->dsrh_p_r4[i][band];
					in->newton_dptrapdp[band]=dpc*in->srh_p_r1[i][band];
					in->newton_dptrapdn[band]= -dnc*in->srh_p_r3[i][band];

					Rtrapp+=pc*in->srh_p_r1[i][band]-in->srh_p_r2[i][band];
					dJpdxipc+=dpc*in->srh_p_r1[i][band];
					in->newton_dJpdtrapp[band]+=pc*in->dsrh_p_r1[i][band]-in->dsrh_p_r2[i][band];

					Rtrapn+= -(-nc*in->srh_p_r3[i][band]+in->srh_p_r4[i][band]);
					dJdxic-= -(-dnc*in->srh_p_r3[i][band]);
					in->newton_dJdtrapp[band]-= -(-nc*in->dsrh_p_r3[i][band]+in->dsrh_p_r4[i][band]);

					if (in->go_time==TRUE)
					{
						in->newton_dptrap[band]+= -(in->pt[i][band]-in->newton_ptlast[band])/dt;
						in->newton_dptrapdptrap[band]+= -(in->dpt[i][band])/dt;
					}

					in->prelax[i]+=pc*in->srh_p_r1[i][band]-in->srh_p_r2[i][band];
					in->ptrap_to_n[i]+= -(-nc*in->srh_p_r3[i][band]+in->srh_p_r4[i][band]);

					in->pt_r1[i][band]=pc*in->srh_p_r1[i][band];
					in->pt_r2[i][band]=in->srh_p_r2[i][band];
					in->pt_r3[i][band]=nc*in->srh_p_r3[i][band];
					in->pt_r4[i][band]=in->srh_p_r4[i][band];

				}

			}

			//band=0;
			//printf("hereh %le %le %le %le\n",in->Vapplied,pc*in->srh_p_r1[i][band]-in->srh_p_r2[i][band]-nc*in->srh_p_r3[i][band]+in->srh_p_r4[i][band],pc*in->srh_p_r1[i][band]-in->srh_p_r2[i][band],-nc*in->srh_p_r3[i][band]+in->srh_p_r4[i][band]);

			
			in->Rn[i]=Rtrapn;
			in->Rp[i]=Rtrapp;
			//Rtrapp=1e24;
			//Rtrapn=1e24;




			if (i!=0)
			{
				in->Ti[pos]=i;
				in->Tj[pos]=i-1;
				in->Tx[pos]=dphil_d;
				pos++;
				//electron
					in->Ti[pos]=in->ymeshpoints*(1)+i;
					in->Tj[pos]=in->ymeshpoints*(1)+i-1;
					in->Tx[pos]=dJdxil;
					//printf("a %le\n",in->Tx[pos]);
					pos++;

					in->Ti[pos]=in->ymeshpoints*(1)+i;
					in->Tj[pos]=i-1;
					in->Tx[pos]=dJdphil;
					//printf("b %le\n",in->Tx[pos]);
					pos++;

					//hole
					in->Ti[pos]=in->ymeshpoints*(1+1)+i;
					in->Tj[pos]=in->ymeshpoints*(1+1)+i-1;
					in->Tx[pos]=dJpdxipl;
					//printf("c %le\n",in->Tx[pos]);
					pos++;

					in->Ti[pos]=i+in->ymeshpoints*(1+1);
					in->Tj[pos]=i-1;
					in->Tx[pos]=dJpdphil;
					//printf("d %le\n",in->Tx[pos]);
					pos++;

			}

			//getchar();
			if ((in->kl_in_newton==TRUE)&&(in->interfaceleft==TRUE)&&(i==0))
			{
				//printf("%d\n",i);
				//getchar();
			}else
			{
				//phi
				in->Ti[pos]=i;
				in->Tj[pos]=i;
				in->Tx[pos]=dphic_d;
				pos++;
			}


			in->Ti[pos]=i;
			in->Tj[pos]=i+in->ymeshpoints*(1);
			in->Tx[pos]=dphidxic;
			//strcpy(in->Tdebug[pos],"dphidxic");
			pos++;
			
			in->Ti[pos]=i;
			in->Tj[pos]=i+in->ymeshpoints*(1+1);
			in->Tx[pos]=dphidxipc;
			//strcpy(in->Tdebug[pos],"dphidxipc");
			pos++;
			

			//electron

			in->Ti[pos]=in->ymeshpoints*(1)+i;
			in->Tj[pos]=in->ymeshpoints*(1)+i;
			in->Tx[pos]=dJdxic;
			//strcpy(in->Tdebug[pos],"dJdxic");
			pos++;

			in->Ti[pos]=in->ymeshpoints*(1)+i;
			in->Tj[pos]=in->ymeshpoints*(1+1)+i;
			in->Tx[pos]=dJdxipc;
			//strcpy(in->Tdebug[pos],"dJdxipc");
			pos++;

			in->Ti[pos]=in->ymeshpoints*(1)+i;
			in->Tj[pos]=i;
			in->Tx[pos]=dJdphic;
			//strcpy(in->Tdebug[pos],"dJdphic");
			pos++;





			//hole
			in->Ti[pos]=in->ymeshpoints*(1+1)+i;
			in->Tj[pos]=in->ymeshpoints*(1+1)+i;
			in->Tx[pos]=dJpdxipc;
			pos++;

			in->Ti[pos]=in->ymeshpoints*(1+1)+i;
			in->Tj[pos]=in->ymeshpoints*(1)+i;
			in->Tx[pos]=dJpdxic;
			pos++;

			in->Ti[pos]=in->ymeshpoints*(1+1)+i;
			in->Tj[pos]=i;
			in->Tx[pos]=dJpdphic;
			pos++;



			if (in->ntrapnewton==TRUE)
			{
				for (band=0;band<in->srh_bands;band++)
				{
					in->Ti[pos]=in->ymeshpoints*(1+1+1+band)+i;
					in->Tj[pos]=in->ymeshpoints*(1+1+1+band)+i;
					in->Tx[pos]=in->newton_dntrapdntrap[band];
					pos++;

					in->Ti[pos]=in->ymeshpoints*(1+1+1+band)+i;
					in->Tj[pos]=in->ymeshpoints*1+i;
					in->Tx[pos]=in->newton_dntrapdn[band];
					pos++;

					in->Ti[pos]=in->ymeshpoints*(1+1+1+band)+i;
					in->Tj[pos]=in->ymeshpoints*(1+1)+i;
					in->Tx[pos]=in->newton_dntrapdp[band];
					pos++;

					in->Ti[pos]=in->ymeshpoints*(1)+i;
					in->Tj[pos]=in->ymeshpoints*(1+1+1+band)+i;
					in->Tx[pos]=in->newton_dJdtrapn[band];
					pos++;

					in->Ti[pos]=in->ymeshpoints*(1+1)+i;
					in->Tj[pos]=in->ymeshpoints*(1+1+1+band)+i;
					in->Tx[pos]=in->newton_dJpdtrapn[band];
					pos++;

					in->Ti[pos]=i;
					in->Tj[pos]=in->ymeshpoints*(1+1+1+band)+i;
					in->Tx[pos]=in->newton_dphidntrap[band];
					pos++;


				}


			}

			if (in->ptrapnewton==TRUE)
			{
				for (band=0;band<in->srh_bands;band++)
				{
					in->Ti[pos]=in->ymeshpoints*(1+1+1+in->srh_bands+band)+i;
					in->Tj[pos]=in->ymeshpoints*(1+1+1+in->srh_bands+band)+i;
					in->Tx[pos]=in->newton_dptrapdptrap[band];
					pos++;

					in->Ti[pos]=in->ymeshpoints*(1+1+1+in->srh_bands+band)+i;
					in->Tj[pos]=in->ymeshpoints*(1+1)+i;
					in->Tx[pos]=in->newton_dptrapdp[band];
					pos++;

					in->Ti[pos]=in->ymeshpoints*(1+1+1+in->srh_bands+band)+i;
					in->Tj[pos]=in->ymeshpoints*(1)+i;
					in->Tx[pos]=in->newton_dptrapdn[band];
					pos++;

					in->Ti[pos]=in->ymeshpoints*(1+1)+i;
					in->Tj[pos]=in->ymeshpoints*(1+1+1+in->srh_bands+band)+i;
					in->Tx[pos]=in->newton_dJpdtrapp[band];
					pos++;

					in->Ti[pos]=in->ymeshpoints*(1)+i;
					in->Tj[pos]=in->ymeshpoints*(1+1+1+in->srh_bands+band)+i;
					in->Tx[pos]=in->newton_dJdtrapp[band];
					pos++;

					in->Ti[pos]=i;
					in->Tj[pos]=in->ymeshpoints*(1+1+1+in->srh_bands+band)+i;
					in->Tx[pos]=in->newton_dphidptrap[band];
					pos++;
				}

			}

			if (i!=(in->ymeshpoints-1))
			{

				if ((in->kl_in_newton==TRUE)&&(in->interfaceleft==TRUE)&&(i==0))
				{
					//printf("%d\n",i);
					//getchar();
				}else
				{
					//phi
					in->Ti[pos]=i;
					in->Tj[pos]=i+1;
					in->Tx[pos]=dphir_d;
					pos++;
				}



				//electron
				in->Ti[pos]=in->ymeshpoints*(1)+i;
				in->Tj[pos]=in->ymeshpoints*(1)+i+1;
				in->Tx[pos]=dJdxir;
				pos++;

				in->Ti[pos]=i+in->ymeshpoints*(1);
				in->Tj[pos]=i+1;
				in->Tx[pos]=dJdphir;
				pos++;

				//hole
				in->Ti[pos]=in->ymeshpoints*(1+1)+i;
				in->Tj[pos]=in->ymeshpoints*(1+1)+i+1;
				in->Tx[pos]=dJpdxipr;
				pos++;

				in->Ti[pos]=i+in->ymeshpoints*(1+1);
				in->Tj[pos]=i+1;
				in->Tx[pos]=dJpdphir;
				pos++;




			}

			//Possion
			gdouble build=0.0;
			if ((in->interfaceleft==TRUE)&&(i==0))
			{
				build= -0.0;
			}else
			if ((in->interfaceright==TRUE)&&(i==in->ymeshpoints-1))
			{
				build= -0.0;
			}else
			{
				build= -(deriv);

				build+= -(-(pc-nc-Nad)*Q);
		
				for (band=0;band<in->srh_bands;band++)
				{
					build+= -(-Q*(in->pt[i][band]-in->nt[i][band]));
				}

				//build+= -(-Q*in->Nad[i]);
				//printf("n=%Le n0=%Le\n",in->n[i],in->n[i]);
				//printf("p=%Le p0=%Le\n",in->p[i],in->p[i]);
				//printf("Nad=%Le Nad0=%Le\n",in->Nad[i],in->Nad[i]);
				//printf("deriv_norm=%Le deriv=%Le\n",deriv,deriv);
			}
			in->b[i]=build;
			//getchar();
			//printf("%Le %Le\n",pc,nc);
			//getchar();
			build=0.0;
			build= -((Jnr-Jnl)/(dylh+dyrh)-Rtrapn-Rfree);


			//printf("p=%Le %Le %Le\n",pl,pc,pr);
			//printf("p=%Le %Le %Le\n",nl,nc,nr);
			//getchar();

			if (in->go_time==TRUE)
			{
				build-= -(nc-nlast)/dt;
			}

			//printf("2 %le\n",in->b[in->ymeshpoints*(1)+i]);
			//getchar();

			//getchar();
			build-=Gn;
			in->b[in->ymeshpoints*(1)+i]=build;
			//printf("3 %Le %le\n",Gn,in->b[in->ymeshpoints*(1)+i]);
			//getchar();

			//hole
			build=0.0;
			build= -((Jpr-Jpl)/(dylh+dyrh)+Rtrapp+Rfree);
			//printf("%le %le\n",Rtrapn,Rtrapp);

			build-= -Gp;

			if (in->go_time==TRUE)
			{
				build-=(pc-plast)/dt;
			}			

			in->b[in->ymeshpoints*(1+1)+i]=build;

			if (in->ntrapnewton==TRUE)
			{
				for (band=0;band<in->srh_bands;band++)
				{
					in->b[in->ymeshpoints*(1+1+1+band)+i]= -(in->newton_dntrap[band]);
				}
			}

			if (in->ptrapnewton==TRUE)
			{
				for (band=0;band<in->srh_bands;band++)
				{
					in->b[in->ymeshpoints*(1+1+1+in->srh_bands+band)+i]= -(in->newton_dptrap[band]);
				}

			}

		}


if (pos>in->N)
{
	ewe(sim,"Error %d %d %d\n",pos,in->N,in->kl_in_newton);
}

//solver_dump_matrix_comments(in->M,pos,in->Tdebug,in->Ti,in->Tj, in->Tx,in->b,"");
//getchar();
//fclose(file_j);
//printf("Check J file\n");
//getchar();

}

gdouble get_cur_error(struct simulation *sim,struct device *in)
{
int i;
gdouble phi=0.0;
gdouble n=0.0;
gdouble p=0.0;
gdouble x=0.0;
gdouble te=0.0;
gdouble th=0.0;
gdouble tl=0.0;
gdouble ttn=0.0;
gdouble ttp=0.0;
gdouble i0=0.0;
int band=0;
for (i=0;i<in->ymeshpoints;i++)
{
		if ((in->interfaceleft==TRUE)&&(i==0))
		{
		}else
		if ((in->interfaceright==TRUE)&&(i==in->ymeshpoints-1))
		{
		}else
		{
			phi+=gfabs(in->b[i]);
			//printf("%Le\n",in->b[i]);
		}

		n+=gfabs(in->b[in->ymeshpoints*(1)+i]);
		p+=+gfabs(in->b[in->ymeshpoints*(1+1)+i]);

		if (in->ntrapnewton==TRUE)
		{
			for (band=0;band<in->srh_bands;band++)
			{
				ttn+=gfabs(in->b[in->ymeshpoints*(1+1+1+band)+i]);
				//printf("error %le\n",fabs(in->b[in->ymeshpoints*(1+1+1+band)+i]));
			}
		}

		if (in->ptrapnewton==TRUE)
		{
			for (band=0;band<in->srh_bands;band++)
			{
				ttp+=gfabs(in->b[in->ymeshpoints*(1+1+1+in->srh_bands+band)+i]);
			}
		}


}
gdouble tot=phi+n+p+x+te+th+tl+ttn+ttp+i0;
//printf("%Le %Le %Le %Le %Le %Le %Le %Le %Le\n",phi,n,p,x,te,th,tl,ttn,ttp);
if (isnan( tot))
{
	printf("%Le %Le %Le %Le %Le %Le %Le %Le %Le\n",phi,n,p,x,te,th,tl,ttn,ttp);
	dump_matrix(sim,in->M,in->N,in->Ti,in->Tj, in->Tx,in->b,"");
	ewe(sim,"nan detected in newton solver\n");
}

return tot;
}

void solver_cal_memory(struct device *in,int *ret_N,int *ret_M)
{
int i=0;
int N=0;
int M=0;
N=in->ymeshpoints*3-2;	//Possion main

N+=in->ymeshpoints*3-2;	//Je main
N+=in->ymeshpoints*3-2;	//Jh main

N+=in->ymeshpoints*3-2;	//dJe/phi
N+=in->ymeshpoints*3-2;	//dJh/phi

N+=in->ymeshpoints;		//dphi/dn
N+=in->ymeshpoints;		//dphi/dh

N+=in->ymeshpoints;		//dJndp
N+=in->ymeshpoints;		//dJpdn

M=in->ymeshpoints;	//Pos

M+=in->ymeshpoints;		//Je
M+=in->ymeshpoints;		//Jh

if (in->ntrapnewton==TRUE)
{
	for (i=0;i<in->srh_bands;i++)
	{
		N+=in->ymeshpoints;		//dntrapdn
		N+=in->ymeshpoints;		//dntrapdntrap
		N+=in->ymeshpoints;		//dntrapdp
		N+=in->ymeshpoints;		//dJndtrapn
		N+=in->ymeshpoints;		//dJpdtrapn
		N+=in->ymeshpoints;		//dphidntrap

		M+=in->ymeshpoints;		//nt
	}

}

if (in->ptrapnewton==TRUE)
{
	for (i=0;i<in->srh_bands;i++)
	{
		N+=in->ymeshpoints;		//dptrapdp
		N+=in->ymeshpoints;		//dptrapdptrap
		N+=in->ymeshpoints;		//dptrapdn
		N+=in->ymeshpoints;		//dJpdtrapp
		N+=in->ymeshpoints;		//dJdtrapp
		N+=in->ymeshpoints;		//dphidptrap

		M+=in->ymeshpoints;		//pt
	}
}
*ret_N=N;
*ret_M=M;
}

void dllinternal_solver_realloc(struct simulation *sim,struct device *in)
{
int N=0;
int M=0;
gdouble *dtemp=NULL;
int *itemp=NULL;

solver_cal_memory(in,&N,&M);


int alloc=FALSE;
if ((in->N==0)||(in->M==0))
{
	in->N=N;
	in->M=M;
	alloc=TRUE;
}else
if ((N!=in->N)||(M!=in->M))
{
	in->N=N;
	in->M=M;
	alloc=TRUE;
}


if (alloc==TRUE)
{

	itemp = realloc(in->Ti,in->N*sizeof(int));
	if (itemp==NULL)
	{
		ewe(sim,"in->Ti - memory error\n");
	}else
	{
		in->Ti=itemp;
	}

	itemp = realloc(in->Tj,in->N*sizeof(int));
	if (itemp==NULL)
	{
		ewe(sim,"in->Tj - memory error\n");
	}else
	{
		in->Tj=itemp;
	}

	dtemp = realloc(in->Tx,in->N*sizeof(gdouble));
	if (dtemp==NULL)
	{
		ewe(sim,"in->Tx - memory error\n");
	}else
	{
		in->Tx=dtemp;
	}

	//int i=0;
	//in->Tdebug = (char**)malloc(in->N*sizeof(char*));

	//for (i=0;i<in->N;i++)
	//{
	//	in->Tdebug[i]= (char*)malloc(20*sizeof(char));
	//	strcpy(in->Tdebug[i],"");
	//}


	dtemp = realloc(in->b,in->M*sizeof(gdouble));

	if (dtemp==NULL)
	{
		ewe(sim,"in->b - memory error\n");
	}else
	{
		in->b=dtemp;
	}

	if (in->srh_bands>0)
	{
		dtemp=realloc(in->newton_dntrap,in->srh_bands*sizeof(gdouble));
		if (dtemp==NULL)
		{
			ewe(sim,"memory error\n");
		}else
		{
			in->newton_dntrap=dtemp;
		}

		dtemp=realloc(in->newton_dntrapdntrap,in->srh_bands*sizeof(gdouble));
		if (dtemp==NULL)
		{
			ewe(sim,"memory error\n");
		}else
		{
			in->newton_dntrapdntrap=dtemp;
		}

		dtemp=realloc(in->newton_dntrapdn,in->srh_bands*sizeof(gdouble));
		if (dtemp==NULL)
		{
			ewe(sim,"memory error\n");
		}else
		{
			in->newton_dntrapdn=dtemp;
		}

		dtemp=realloc(in->newton_dntrapdp,in->srh_bands*sizeof(gdouble));
		if (dtemp==NULL)
		{
			ewe(sim,"memory error\n");
		}else
		{
			in->newton_dntrapdp=dtemp;
		}

		dtemp=realloc(in->newton_dJdtrapn,in->srh_bands*sizeof(gdouble));
		if (dtemp==NULL)
		{
			ewe(sim,"memory error\n");
		}else
		{
			in->newton_dJdtrapn=dtemp;
		}

		dtemp=realloc(in->newton_dJpdtrapn,in->srh_bands*sizeof(gdouble));
		if (dtemp==NULL)
		{
			ewe(sim,"memory error\n");
		}else
		{
			in->newton_dJpdtrapn=dtemp;
		}

		dtemp=realloc(in->newton_dphidntrap,in->srh_bands*sizeof(gdouble));
		if (dtemp==NULL)
		{
			ewe(sim,"memory error\n");
		}else
		{
			in->newton_dphidntrap=dtemp;
		}


		dtemp=realloc(in->newton_ntlast,in->srh_bands*sizeof(gdouble));
		if (dtemp==NULL)
		{
			ewe(sim,"memory error\n");
		}else
		{
			in->newton_ntlast=dtemp;
		}



		dtemp=realloc(in->newton_dptrapdp,in->srh_bands*sizeof(gdouble));
		if (dtemp==NULL)
		{
			ewe(sim,"memory error\n");
		}else
		{
			in->newton_dptrapdp=dtemp;
		}

		dtemp=realloc(in->newton_dptrapdptrap,in->srh_bands*sizeof(gdouble));
		if (dtemp==NULL)
		{
			ewe(sim,"memory error\n");
		}else
		{
			in->newton_dptrapdptrap=dtemp;
		}

		dtemp=realloc(in->newton_dptrap,in->srh_bands*sizeof(gdouble));
		if (dtemp==NULL)
		{
			ewe(sim,"memory error\n");
		}else
		{
			in->newton_dptrap=dtemp;
		}

		dtemp=realloc(in->newton_dptrapdn,in->srh_bands*sizeof(gdouble));
		if (dtemp==NULL)
		{
			ewe(sim,"memory error\n");
		}else
		{
			in->newton_dptrapdn=dtemp;
		}

		dtemp=realloc(in->newton_dJpdtrapp,in->srh_bands*sizeof(gdouble));
		if (dtemp==NULL)
		{
			ewe(sim,"memory error\n");
		}else
		{
			in->newton_dJpdtrapp=dtemp;
		}


		dtemp=realloc(in->newton_dJdtrapp,in->srh_bands*sizeof(gdouble));
		if (dtemp==NULL)
		{
			ewe(sim,"memory error\n");
		}else
		{
			in->newton_dJdtrapp=dtemp;
		}

		dtemp=realloc(in->newton_dphidptrap,in->srh_bands*sizeof(gdouble));
		if (dtemp==NULL)
		{
			ewe(sim,"memory error\n");
		}else
		{
			in->newton_dphidptrap=dtemp;
		}

		dtemp=realloc(in->newton_ptlast,in->srh_bands*sizeof(gdouble));
		if (dtemp==NULL)
		{
			ewe(sim,"memory error\n");
		}else
		{
			in->newton_ptlast=dtemp;
		}

	}

}

}

void dllinternal_solver_free_memory(struct device *in)
{
//int i=0;

if (in->srh_bands>0)
{
	free(in->newton_dntrap);
	free(in->newton_dntrapdntrap);
	free(in->newton_dntrapdn);
	free(in->newton_dntrapdp);
	free(in->newton_dJdtrapn);
	free(in->newton_dJpdtrapn);
	free(in->newton_dphidntrap);
	free(in->newton_dptrapdp);
	free(in->newton_dptrapdptrap);
	free(in->newton_dptrap);
	free(in->newton_dptrapdn);
	free(in->newton_dJpdtrapp);
	free(in->newton_dJdtrapp);
	free(in->newton_dphidptrap);

	free(in->newton_ntlast);
	free(in->newton_ptlast);
}

free(in->Ti);
free(in->Tj);
free(in->Tx);
free(in->b);

//for (i=0;i<in->N;i++)
//{
//	free(in->Tdebug[i]);
//}
//free(in->Tdebug);

in->newton_dntrapdntrap=NULL;
in->newton_dntrap=NULL;
in->newton_dntrapdn=NULL;
in->newton_dntrapdp=NULL;
in->newton_dJdtrapn=NULL;
in->newton_dJpdtrapn=NULL;
in->newton_dphidntrap=NULL;
in->newton_dptrapdp=NULL;
in->newton_dptrapdptrap=NULL;
in->newton_dptrap=NULL;
in->newton_dptrapdn=NULL;
in->newton_dJpdtrapp=NULL;
in->newton_dJdtrapp=NULL;
in->newton_dphidptrap=NULL;


in->newton_ntlast=NULL;
in->newton_ptlast=NULL;

in->Ti=NULL;
in->Tj=NULL;
in->Tx=NULL;
in->b=NULL;
in->Tdebug=NULL;

}

int dllinternal_solve_cur(struct simulation *sim,struct device *in)
{
gdouble error=0.0;
int ittr=0;
if (get_dump_status(sim,dump_print_newtonerror)==TRUE)
{
	printf("Solve cur\n");
}




#ifdef only
only_update_thermal=FALSE;
#endif
//in->enable_back=FALSE;
int stop=FALSE;
int thermalrun=0;
gdouble check[10];
int cpos=0;
//for (i=0;i<in->ymeshpoints;i++)
//{
//	printf("Rod ------- nt= %d %le\n",i,in->Gn[i]);
//}
	do
	{
		fill_matrix(sim,in);


//dump_for_plot(in);
//plot_now(in,"plot");
	//solver_dump_matrix(in->M,in->N,in->Ti,in->Tj, in->Tx,in->b);
//getchar();

			if (in->stop==TRUE)
			{
				break;
			}

			solver(sim,in->M,in->N,in->Ti,in->Tj, in->Tx,in->b);			

			update_solver_vars(sim,in,TRUE);

			//printf("Going to clamp=%d\n",proper);
			//solver_dump_matrix(in->M,in->N,in->Ti,in->Tj, in->Tx,in->b);
			//printf("%d\n");
			//getchar();	
		

		error=get_cur_error(sim,in);

		//thermalrun++;
		if (thermalrun==40) thermalrun=0;
		//update(in);
//getchar();

		if (get_dump_status(sim,dump_print_newtonerror)==TRUE)
		{
			printf("%d Cur error = %Le %Le I=%Le",ittr,error,in->Vapplied,get_I(in));
		
			printf("\n");
		}

		in->last_error=error;
		in->last_ittr=ittr;
		ittr++;

		if (get_dump_status(sim,dump_write_converge)==TRUE)
		{
			sim->converge=fopena(get_output_path(sim),"converge.dat","a");
			fprintf(sim->converge,"%Le\n",error);
			fclose(sim->converge);
		}

		stop=TRUE;

		if (ittr<in->max_electrical_itt)
		{
			if (error>in->min_cur_error)
			{
				stop=FALSE;
			}
		}

		if (ittr<in->newton_min_itt)
		{
			stop=FALSE;
		}


		if (in->newton_clever_exit==TRUE)
		{
			check[cpos]=error;
			cpos++;

			if (cpos>10)
			{
				cpos=0;
			}

			if (ittr>=in->newton_min_itt)
			{
					if ((check[0]<error)||(check[1]<error))
					{
						stop=TRUE;
					}	
			}
		}



	}while(stop==FALSE);

in->newton_last_ittr=ittr;

if (error>1e-3)
{
	printf_log(sim,"warning: The solver has not converged very well.\n");
}

//getchar();
if (get_dump_status(sim,dump_newton)==TRUE)
{
	dump_1d_slice(sim,in,get_output_path(sim));
}
//plot_now(in,"plot");
//getchar();
in->odes+=in->M;
//getchar();

return 0;
}


