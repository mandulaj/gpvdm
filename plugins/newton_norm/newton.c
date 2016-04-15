//
//  General-purpose Photovoltaic Device Model gpvdm.com- a drift diffusion
//  base/Shockley-Read-Hall model for 1st, 2nd and 3rd generation solarcells.
// 
//  Copyright (C) 2012 Roderick C. I. MacKenzie
//
//      roderick.mackenzie@nottingham.ac.uk
//      www.roderickmackenzie.eu
//      Room B86 Coates, University Park, Nottingham, NG7 2RD, UK
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

#include <stdlib.h>
#include <string.h>
#include <const.h>
#include "newton.h"
#include <dll_export.h>
#include <log.h>
#include <exp.h>
#include <util.h>
#include <dump.h>
#include <cal_path.h>

static gdouble Jnl = 0.0;
static gdouble Jnr = 0.0;
static gdouble Jpl = 0.0;
static gdouble Jpr = 0.0;

static gdouble Dnl = 0.0;
static gdouble Dnc = 0.0;
static gdouble Dnr = 0.0;
static gdouble Dpl = 0.0;
static gdouble Dpc = 0.0;
static gdouble Dpr = 0.0;

static gdouble nl = 0.0;
static gdouble nc = 0.0;
static gdouble nr = 0.0;

static gdouble pl = 0.0;
static gdouble pc = 0.0;
static gdouble pr = 0.0;

static gdouble xil = 0.0;
static gdouble xir = 0.0;
static gdouble xipl = 0.0;
static gdouble xipr = 0.0;

static gdouble dJpdxipl = 0.0;
static gdouble dJpdxipc = 0.0;
static gdouble dJpdxipr = 0.0;

static gdouble dnl = 0.0;
static gdouble dnc = 0.0;
static gdouble dnr = 0.0;

static gdouble dpl = 0.0;
static gdouble dpc = 0.0;
static gdouble dpr = 0.0;

static gdouble munl = 0.0;
static gdouble munc = 0.0;
static gdouble munr = 0.0;

static gdouble mupl = 0.0;
static gdouble mupc = 0.0;
static gdouble mupr = 0.0;

static gdouble wnl = 0.0;
static gdouble wnc = 0.0;
static gdouble wnr = 0.0;

static gdouble wpl = 0.0;
static gdouble wpc = 0.0;
static gdouble wpr = 0.0;

static gdouble dJdxil = 0.0;
static gdouble dJdxic = 0.0;
static gdouble dJdxir = 0.0;

static gdouble dJdphil = 0.0;
static gdouble dJdphic = 0.0;
static gdouble dJdphir = 0.0;

static gdouble dJpdphil = 0.0;
static gdouble dJpdphic = 0.0;
static gdouble dJpdphir = 0.0;

static gdouble dphidxic = 0.0;
static gdouble dphidxipc = 0.0;

static gdouble *dntrap = NULL;
static gdouble *dntrapdntrap = NULL;
static gdouble *dntrapdn = NULL;
static gdouble *dntrapdp = NULL;
static gdouble *dJdtrapn = NULL;
static gdouble *dJpdtrapn = NULL;

static gdouble *dptrapdp = NULL;
static gdouble *dptrapdptrap = NULL;
static gdouble *dptrap = NULL;
static gdouble *dptrapdn = NULL;
static gdouble *dJpdtrapp = NULL;
static gdouble *dJdtrapp = NULL;
static gdouble *dphidntrap = NULL;
static gdouble *dphidptrap = NULL;
static int newton_min_ittr;

#define D0 (gdouble)243.75
#define n0 (gdouble)1e27
#define phi0 (gdouble)(kb*300e4/Q)
#define l0 ((gdouble)sqrtl(epsilon0*kb*300e4/Q/Q/n0))
#define r0 ((gdouble)n0*D0/l0/l0)
#define r_bi0 ((gdouble)(D0/(n0*l0*l0)))

void dllinternal_newton_set_min_ittr(int ittr)
{
	newton_min_ittr = ittr;
}

void update_solver_vars(struct simulation *sim, struct device *in, int clamp)
{
	int i;
	int band = 0;

//printf("%Le\n",in->Gn[0]);
	gdouble clamp_temp = 300.0;

	gdouble update = 0.0;
	for (i = 0; i < in->ymeshpoints; i++) {

		update = (gdouble) in->b[i] * phi0;
		if ((in->interfaceleft == TRUE) && (i == 0)) {
		} else
		    if ((in->interfaceright == TRUE)
			&& (i == in->ymeshpoints - 1)) {
		} else {
			if (clamp == TRUE) {
				in->phi[i] +=
				    update / (1.0 +
					      gfabs(update /
						    in->electrical_clamp /
						    (clamp_temp * kb / Q)));
			} else {
				in->phi[i] += update;

			}
		}

		update = (gdouble) (in->b[in->ymeshpoints * (1) + i]) * phi0;
		if (clamp == TRUE) {
			in->x[i] +=
			    update / (1.0 +
				      gfabs(update / in->electrical_clamp /
					    (clamp_temp * kb / Q)));
		} else {
			in->x[i] += update;
		}

		update =
		    (gdouble) (in->b[in->ymeshpoints * (1 + 1) + i]) * phi0;
		if (clamp == TRUE) {
			in->xp[i] +=
			    update / (1.0 +
				      gfabs(update / in->electrical_clamp /
					    (clamp_temp * kb / Q)));
		} else {
			in->xp[i] += update;

		}

		if (in->ntrapnewton == TRUE) {
			for (band = 0; band < in->srh_bands; band++) {
				update =
				    (gdouble) (in->
					       b[in->ymeshpoints *
						 (1 + 1 + 1 + band) + i]);
				if (clamp == TRUE) {
					in->xt[i][band] +=
					    update / (1.0 +
						      gfabs(update /
							    in->
							    electrical_clamp /
							    (clamp_temp * kb /
							     Q)));

				} else {
					in->xt[i][band] += update;
				}
			}
		}

		if (in->ptrapnewton == TRUE) {
			for (band = 0; band < in->srh_bands; band++) {
				update =
				    (gdouble) (in->
					       b[in->ymeshpoints *
						 (1 + 1 + 1 + in->srh_bands +
						  band) + i]);
				if (clamp == TRUE) {
					in->xpt[i][band] +=
					    update / (1.0 +
						      gfabs(update /
							    in->
							    electrical_clamp /
							    (clamp_temp * kb /
							     Q)));
				} else {
					in->xpt[i][band] += update;

				}
			}
		}

	}

	(*fun->update_arrays) (sim, in);

}

void fill_matrix(struct simulation *sim, struct device *in)
{
//gdouble offset= -0.5;
	int band = 0;

	(*fun->update_arrays) (sim, in);

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
	gdouble ddh = 0.0;
//gdouble dh;
	int pos = 0;

	gdouble Ecl = 0.0;
	gdouble Ecr = 0.0;
	gdouble Ecc = 0.0;
	gdouble Evl = 0.0;
	gdouble Evr = 0.0;
	gdouble Evc = 0.0;

	gdouble Tel = 0.0;
//gdouble Tec=0.0;
	gdouble Ter = 0.0;

	gdouble Thl = 0.0;
//gdouble Thc=0.0;
	gdouble Thr = 0.0;

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
	gdouble dJdxipc = 0.0;
	gdouble dJpdxic = 0.0;

	gdouble e0 = 0.0;
	gdouble e1 = 0.0;

	gdouble dphil = 0.0;
	gdouble dphic = 0.0;
	gdouble dphir = 0.0;
	gdouble deriv = 0.0;

//gdouble kll=0.0;
//gdouble klc=0.0;
//gdouble klr=0.0;

//gdouble kl0=0.0;
//gdouble kl1=0.0;

	gdouble one = 0.0;
	gdouble one0_l = 0.0;
	gdouble one0_r = 0.0;

	gdouble Rtrapn = 0.0;
	gdouble Rtrapp = 0.0;

	gdouble dJdphil_leftl = 0.0;
	gdouble dJdphil_leftc = 0.0;
	gdouble dJpdphil_leftl = 0.0;
	gdouble dJpdphil_leftc = 0.0;
	gdouble dphil_left = 0.0;
	gdouble dJdxil_leftc = 0.0;
	gdouble dJpdxipl_leftc = 0.0;

	gdouble dJdxic_rightc = 0.0;
	gdouble dJpdxipc_rightc = 0.0;
	gdouble dJpdphi_rightc = 0.0;
	gdouble dJdphi_rightc = 0.0;

	gdouble Bfree = 0.0;
	gdouble nceq = 0.0;
	gdouble pceq = 0.0;
	gdouble Rfree = 0.0;

	gdouble nc0_l = 0.0;
//gdouble dnc0_l=0.0;
//gdouble pc0_l=0.0;
//gdouble dpc0_l=0.0;

	gdouble nc0_r = 0.0;
//gdouble dnc0_r=0.0;
//gdouble pc0_r=0.0;
//gdouble dpc0_r=0.0;

	gdouble dJnldxil_l = 0.0;
	gdouble dJnldxil_c = 0.0;
	gdouble dJnrdxir_c = 0.0;
	gdouble dJnrdxir_r = 0.0;
	gdouble dJpldxipl_l = 0.0;
	gdouble dJpldxipl_c = 0.0;
	gdouble dJprdxipr_c = 0.0;
	gdouble dJprdxipr_r = 0.0;

	gdouble i0 = 0.0;
	gdouble didv = 0.0;	//not a function
	gdouble diphic = 0.0;	//could be a function
	gdouble didxic = 0.0;
	gdouble didxipc = 0.0;
	gdouble didphir = 0.0;
	gdouble didxir = 0.0;
	gdouble didxipr = 0.0;
	gdouble Nad = 0.0;

//gdouble dylh_left=0.0;
//gdouble dyrh_left=0.0;
//gdouble dncdphic=0.0;
//gdouble dpcdphic=0.0;

	if (in->kl_in_newton == FALSE) {
		if (in->interfaceleft == TRUE) {
			in->phi[0] = in->Vapplied;
		}
	}

	if (in->interfaceright == TRUE) {
		in->phi[in->ymeshpoints - 1] = in->Vr;
	}

	pos = 0;
	for (i = 0; i < in->ymeshpoints; i++) {
		if (i == 0) {
//                              exl=0.0;
//                              Dexl=in->Dex[0];

			phil = (in->Vapplied / phi0);
			//printf("setr=%Lf %Lf\n",phil,in->Vapplied);

			yl = in->ymesh[0] / l0 - (in->ymesh[1] -
						  in->ymesh[0]) / l0;
//                              Tll=in->Tll;
			Tel = in->Tll;
			Thl = in->Tll;

			Ecl = -in->Xi[0] / phi0 - phil;
			Evl = -in->Xi[0] / phi0 - phil - in->Eg[0] / phi0;
			epl = in->epsilonr[0];	//*epsilon0;

			xnl = in->Fi[0] / phi0;
			tnl = in->Xi[0] / phi0;
			one = xnl + tnl;

			nl = (*fun->get_n_den) (one * phi0, Tel,
						in->imat[i]) / n0;
			dnl =
			    (*fun->get_dn_den) (one * phi0, Tel,
						in->imat[i]) * phi0 / n0;
			wnl = (*fun->get_n_w) (one * phi0, Tel, in->imat[i]);

			munl = in->mun[0];

			xpl = -in->Fi[0] / phi0;
			tpl = (in->Xi[0] / phi0 + in->Eg[0] / phi0);
			one = xpl - tpl;

			pl = (*fun->get_p_den) (one * phi0, Thl,
						in->imat[i]) / n0;
			dpl =
			    (*fun->get_dp_den) (one * phi0, Thl,
						in->imat[i]) * phi0 / n0;
			wpl = (*fun->get_p_w) (one * phi0, Thl, in->imat[i]);

			mupl = in->mup[0];

			//printf("left n= %Le p= %Le \n",nl,pl);

//                              kll=in->kl[i];

		} else {
//                              Dexl=in->Dex[i-1];
//                              exl=in->ex[i-1];                                
			phil = in->phi[i - 1] / phi0;
			yl = in->ymesh[i - 1] / l0;
//                              Tll=in->Tl[i-1];
			Tel = in->Te[i - 1];
			Thl = in->Th[i - 1];
			Ecl = in->Ec[i - 1] / phi0;
			Evl = in->Ev[i - 1] / phi0;

			nl = in->n[i - 1] / n0;
			dnl = in->dn[i - 1] * phi0 / n0;

			wnl = in->wn[i - 1];
			wpl = in->wp[i - 1];

			pl = in->p[i - 1] / n0;
			dpl = in->dp[i - 1] * phi0 / n0;
			munl = in->mun[i - 1];
			mupl = in->mup[i - 1];

			epl = in->epsilonr[i - 1];	//*epsilon0;

//                              kll=in->kl[i-1];
		}

		Ecc = (-in->Xi[i] / phi0 - in->phi[i] / phi0);
		Evc =
		    (-in->Xi[i] / phi0 - in->phi[i] / phi0 - in->Eg[i] / phi0);

		if (i == (in->ymeshpoints - 1)) {

//                              Dexr=in->Dex[i];
//                              exr=0.0;                        
			//phir=in->Vr;

			phir = in->Vr / phi0;

			yr = in->ymesh[i] / l0 + (in->ymesh[i] -
						  in->ymesh[i - 1]) / l0;
//                              Tlr=in->Tlr;
			Ter = in->Tlr;
			Thr = in->Tlr;

			Ecr = -in->Xi[i] / phi0 - phir;
			Evr = -in->Xi[i] / phi0 - phir - in->Eg[i] / phi0;

			xnr = (in->Vr / phi0 + in->Fi[i] / phi0);
			tnr = (in->Xi[i] / phi0);

			one = xnr + tnr;

			nr = (*fun->get_n_den) (one * phi0, Ter,
						in->imat[i]) / n0;
			dnr =
			    (*fun->get_dn_den) (one * phi0, Ter,
						in->imat[i]) * phi0 / n0;
			wnr = (*fun->get_n_w) (one * phi0, Ter, in->imat[i]);

			xpr = -(in->Vr / phi0 + in->Fi[i] / phi0);
			tpr = (in->Xi[i] / phi0 + in->Eg[i] / phi0);

			one = xpr - tpr;

			pr = (*fun->get_p_den) (one * phi0, Thr,
						in->imat[i]) / n0;
			dpr =
			    (*fun->get_dp_den) (one * phi0, Thr,
						in->imat[i]) * phi0 / n0;
			wpr = (*fun->get_p_w) (one * phi0, Thr, in->imat[i]);

			munr = in->mun[i];
			mupr = in->mup[i];

			//printf("right n= %Le p= %Le \n",nr,pr);

			epr = in->epsilonr[i];	//*epsilon0;
//                              klr=in->kl[i];

		} else {

//                              Dexr=in->Dex[i+1];
//                              exr=in->ex[i+1];                                
			phir = in->phi[i + 1] / phi0;
			yr = in->ymesh[i + 1] / l0;
//                              Tlr=in->Tl[i+1];
			Ter = in->Te[i + 1];
			Thr = in->Th[i + 1];

			Ecr = in->Ec[i + 1] / phi0;
			Evr = in->Ev[i + 1] / phi0;

			nr = in->n[i + 1] / n0;
			dnr = in->dn[i + 1] * phi0 / n0;

			wnr = in->wn[i + 1];
			wpr = in->wp[i + 1];

			pr = in->p[i + 1] / n0;
			dpr = in->dp[i + 1] * phi0 / n0;
			munr = in->mun[i + 1];
			mupr = in->mup[i + 1];

			epr = in->epsilonr[i + 1];	//*epsilon0;
//                              klr=in->kl[i+1];

		}

		dJdxipc = 0.0;
		dJpdxic = 0.0;

		epc = in->epsilonr[i];	//*epsilon0;

//                      exc=in->ex[i];
//                      Dexc=in->Dex[i];
		yc = in->ymesh[i] / l0;
		dyl = yc - yl;
		dyr = yr - yc;
		ddh = (dyl + dyr) / 2.0;
		gdouble dylh = dyl / 2.0;
		gdouble dyrh = dyr / 2.0;

//                      dh=(dyl+dyr);
		phic = in->phi[i] / phi0;
//                      Tlc=in->Tl[i];
//                      Tec=in->Te[i];
//                      Thc=in->Th[i];

		munc = in->mun[i];
		mupc = in->mup[i];

		wnc = in->wn[i];
		wpc = in->wp[i];

		Dnl = munl * (2.0 / 3.0) * wnl / Q;
		Dpl = mupl * (2.0 / 3.0) * wpl / Q;

		Dnc = munc * (2.0 / 3.0) * wnc / Q;
		Dpc = mupc * (2.0 / 3.0) * wpc / Q;
		in->Dn[i] = Dnc;
		in->Dp[i] = Dnc;

		Dnr = munr * (2.0 / 3.0) * wnr / Q;
		Dpr = mupr * (2.0 / 3.0) * wpr / Q;

		Dnl = (Dnl + Dnc) / 2.0;
		Dnr = (Dnr + Dnc) / 2.0;

		Dpl = (Dpl + Dpc) / 2.0;
		Dpr = (Dpr + Dpc) / 2.0;

		munl = (munl + munc) / 2.0;
		munr = (munr + munc) / 2.0;

		mupl = (mupl + mupc) / 2.0;
		mupr = (mupr + mupc) / 2.0;

		nc = in->n[i] / n0;
		pc = in->p[i] / n0;

		dnc = in->dn[i] * phi0 / n0;
		dpc = in->dp[i] * phi0 / n0;
//                              dncdphic=in->dndphi[i];
//                              dpcdphic=in->dpdphi[i];

		Bfree = in->B[i] / r_bi0;
		Nad = in->Nad[i] / n0;
		nceq = in->nfequlib[i] / n0;
		pceq = in->pfequlib[i] / n0;
		Rfree = Bfree * (nc * pc - nceq * pceq);
		in->Rfree[i] = Rfree * r_bi0;
		//if (in->ymeshpoints*(1)+i==31) printf("Rod -- %d %le %le %le \n",in->ymeshpoints*(1)+i,Rfree,nceq,pceq);

//                      klc=in->kl[i];

//      R=in->R[i];
		Gn = in->Gn[i] / r0;
		Gp = in->Gp[i] / r0;

		e0 = (epl + epc) / 2.0;
		e1 = (epc + epr) / 2.0;

//      kl0=(klc+kll)/2.0;
//      kl1=(klr+klc)/2.0;

		dphil = -e0 / dyl / ddh;
		dphic = e0 / dyl / ddh + e1 / dyr / ddh;
		dphir = -e1 / dyr / ddh;

		gdouble dphil_d = dphil;
		gdouble dphic_d = dphic;
		gdouble dphir_d = dphir;

		//if (i==in->ymeshpoints-1)
		//{
		//printf("%le\n",phir);
//
		//}
		deriv = phil * dphil + phic * dphic + phir * dphir;
		//if (in->Vapplied>0.1)
		//{
		//      printf("%Le %Le %Le %Le %Le\n",phil,phic,phir,nc,pc);
		//      getchar();
		//}
		dphidxic = (dnc);	//Q*
		dphidxipc = -(dpc);	//Q*

		if (((in->interfaceleft == TRUE) && (i == 0))
		    || ((in->interfaceright == TRUE)
			&& (i == in->ymeshpoints - 1))) {
			dphidxic = 0.0;
			dphidxipc = 0.0;
		}

		if (in->ntrapnewton == TRUE) {
			for (band = 0; band < in->srh_bands; band++) {
				dphidntrap[band] = Q * (in->dnt[i][band]);
				if ((in->interfaceleft == TRUE) && (i == 0))
					dphidntrap[band] = 0.0;
				if ((in->interfaceright == TRUE)
				    && (i == in->ymeshpoints - 1))
					dphidntrap[band] = 0.0;
			}
		}

		if (in->ptrapnewton == TRUE) {
			for (band = 0; band < in->srh_bands; band++) {
				dphidptrap[band] = -Q * (in->dpt[i][band]);
				if ((in->interfaceleft == TRUE) && (i == 0))
					dphidptrap[band] = 0.0;
				if ((in->interfaceright == TRUE)
				    && (i == in->ymeshpoints - 1))
					dphidptrap[band] = 0.0;

				//dphidxipc+= -Q*(in->dpt[i]);
			}
		}

//      G=in->G[i];

		xil =
		    Q * 2.0 * (3.0 / 2.0) * phi0 * (Ecc - Ecl) / ((wnc + wnl));
		xir =
		    Q * 2.0 * (3.0 / 2.0) * phi0 * (Ecr - Ecc) / ((wnr + wnc));

		//gdouble dxil= -Q*2.0*(3.0/2.0)*(Ecc-Ecl)/pow((wnc+wnl),2.0);
		//gdouble dxir= -Q*2.0*(3.0/2.0)*(Ecr-Ecc)/pow((wnr+wnc),2.0);

		xipl = Q * 2.0 * (3.0 / 2.0) * phi0 * (Evc - Evl) / (wpc + wpl);
		xipr = Q * 2.0 * (3.0 / 2.0) * phi0 * (Evr - Evc) / (wpr + wpc);

		dJdxil = 0.0;
		dJdxic = 0.0;
		dJdxir = 0.0;

		dJpdxipl = 0.0;
		dJpdxipc = 0.0;
		dJpdxipr = 0.0;

		dJdphil = 0.0;
		dJdphic = 0.0;
		dJdphir = 0.0;

		dJpdphil = 0.0;
		dJpdphic = 0.0;
		dJpdphir = 0.0;

		Jnl = (Dnl / D0 / dyl) * (B(-xil) * nc - B(xil) * nl);
		dJnldxil_l = -(Dnl / D0 / dyl) * (B(xil) * dnl);
		dJnldxil_c = (Dnl / D0 / dyl) * B(-xil) * dnc;

		gdouble dJnldphi_l =
		    -phi0 * (munl / D0 / dyl) * (dB(-xil) * nc + dB(xil) * nl);
		gdouble dJnldphi_c =
		    phi0 * (munl / D0 / dyl) * (dB(-xil) * nc + dB(xil) * nl);

		Jnr = (Dnr / D0 / dyr) * (B(-xir) * nr - B(xir) * nc);
		dJnrdxir_c = -(Dnr / D0 / dyr) * (B(xir) * dnc);
		dJnrdxir_r = (Dnr / D0 / dyr) * (B(-xir) * dnr);

		gdouble dJnrdphi_c =
		    phi0 * (munr / D0 / dyr) * (-dB(-xir) * nr - dB(xir) * nc);
		gdouble dJnrdphi_r =
		    phi0 * (munr / D0 / dyr) * (dB(-xir) * nr + dB(xir) * nc);

		Jpl = (Dpl / D0 / dyl) * (B(-xipl) * pl - B(xipl) * pc);
		dJpldxipl_l = (Dpl / D0 / dyl) * (B(-xipl) * dpl);
		dJpldxipl_c = -(Dpl / D0 / dyl) * B(xipl) * dpc;

		gdouble dJpldphi_l =
		    -phi0 * ((mupl / D0) / dyl) * (dB(-xipl) * pl +
						   dB(xipl) * pc);
		gdouble dJpldphi_c =
		    phi0 * ((mupl / D0) / dyl) * (dB(-xipl) * pl +
						  dB(xipl) * pc);

		Jpr = (Dpr / D0 / dyr) * (B(-xipr) * pc - B(xipr) * pr);
		dJprdxipr_c = (Dpr / D0 / dyr) * (B(-xipr) * dpc);
		dJprdxipr_r = -(Dpr / D0 / dyr) * (B(xipr) * dpr);

		gdouble dJprdphi_c =
		    -phi0 * (mupr / D0 / dyr) * (dB(-xipr) * pc +
						 dB(xipr) * pr);
		gdouble dJprdphi_r =
		    phi0 * (mupr / D0 / dyr) * (dB(-xipr) * pc + dB(xipr) * pr);

		if (i == 0) {
			//printf("%le %le %le %le\n",in->vl*(nc-nc0_l),Jnl,-in->vl*(pc-pc0_l),Jpl);
			//getchar();
			in->Jnleft = Jnl * D0 * n0 / l0;
			in->Jpleft = Jpl * D0 * n0 / l0;
		}

		if (i == in->ymeshpoints - 1) {
			//printf("%le %le %le %le\n",in->vr*(nc-nc0_r),Jnr,in->vr*(pc-pc0_r),Jpr);
			in->Jnright = Jnr * D0 * n0 / l0;
			in->Jpright = Jpr * D0 * n0 / l0;
		}
		//printf("----------\n");
		//printf("%le %le\n",Jnl,Jpl);
		//printf("%le %le\n",Jnr,Jpr);

		in->Jn[i] = Q * (Jnl + Jnr) * D0 * n0 / l0 / 2.0;
		in->Jp[i] = Q * (Jpl + Jpr) * D0 * n0 / l0 / 2.0;

		dJdxil += -dJnldxil_l / (dylh + dyrh);
		dJdxic += (-dJnldxil_c + dJnrdxir_c) / (dylh + dyrh);
		dJdxir += dJnrdxir_r / (dylh + dyrh);

		dJpdxipl += -dJpldxipl_l / (dylh + dyrh);
		dJpdxipc += (-dJpldxipl_c + dJprdxipr_c) / (dylh + dyrh);
		dJpdxipr += dJprdxipr_r / (dylh + dyrh);

		dJdphil += -dJnldphi_l / (dylh + dyrh);
		dJdphic += (-dJnldphi_c + dJnrdphi_c) / (dylh + dyrh);
		dJdphir += dJnrdphi_r / (dylh + dyrh);

		dJpdphil += -dJpldphi_l / (dylh + dyrh);
		dJpdphic += (-dJpldphi_c + dJprdphi_c) / (dylh + dyrh);
		dJpdphir += dJprdphi_r / (dylh + dyrh);

		if (Bfree != 0.0) {
			dJdxic += -Bfree * (dnc * pc);
			dJdxipc += -Bfree * (nc * dpc);

			dJpdxipc += Bfree * (nc * dpc);
			dJpdxic += Bfree * (dnc * pc);

			if ((in->interfaceleft == TRUE) && (i == 0)) {
			} else
			    if ((in->interfaceright == TRUE)
				&& (i == in->ymeshpoints - 1)) {
			} else {
				dJdphic += -Bfree * (dnc * pc);
				dJpdphic += Bfree * (nc * dpc);
			}

		}

		if (i == 0) {

			dJdphil_leftl = dJnldphi_l;
			dJdphil_leftc = dJnldphi_c;
			dJpdphil_leftl = dJpldphi_l;
			dJpdphil_leftc = dJpldphi_c;
			//printf("%le %le\n",dpc,dnc);
			dphil_left = -e0 / dyl / ddh;
			dJdxil_leftc = dJnldxil_c;
			dJpdxipl_leftc = dJpldxipl_c;
			//dylh_left=dylh;
			//dyrh_left=dyrh;

		}

		if (i == in->ymeshpoints - 1) {
			dJdxic_rightc = dJnrdxir_c;
			dJpdxipc_rightc = dJprdxipr_c;
			dJdphi_rightc = -dJnrdphi_r;
			dJpdphi_rightc = dJprdphi_c;
		}

		Rtrapn = 0.0;
		Rtrapp = 0.0;

		in->nrelax[i] = 0.0;
		in->ntrap_to_p[i] = 0.0;
		in->prelax[i] = 0.0;
		in->ptrap_to_n[i] = 0.0;

		if (in->ntrapnewton == TRUE) {
			for (band = 0; band < in->srh_bands; band++) {
				dJdtrapn[band] = 0.0;
				dJpdtrapn[band] = 0.0;
				dntrap[band] =
				    nc * in->srh_n_r1[i][band] -
				    in->srh_n_r2[i][band] -
				    pc * in->srh_n_r3[i][band] +
				    in->srh_n_r4[i][band];
				dntrapdntrap[band] =
				    nc * in->dsrh_n_r1[i][band] -
				    in->dsrh_n_r2[i][band] -
				    pc * in->dsrh_n_r3[i][band] +
				    in->dsrh_n_r4[i][band];
				dntrapdn[band] = dnc * in->srh_n_r1[i][band];
				dntrapdp[band] = -dpc * in->srh_n_r3[i][band];
				Rtrapn +=
				    nc * in->srh_n_r1[i][band] -
				    in->srh_n_r2[i][band];
				dJdxic -= dnc * in->srh_n_r1[i][band];
				dJdtrapn[band] -=
				    nc * in->dsrh_n_r1[i][band] -
				    in->dsrh_n_r2[i][band];
				Rtrapp +=
				    -(-pc * in->srh_n_r3[i][band] +
				      in->srh_n_r4[i][band]);
				dJpdxipc += -(-dpc * in->srh_n_r3[i][band]);
				dJpdtrapn[band] =
				    -(-pc * in->dsrh_n_r3[i][band] +
				      in->dsrh_n_r4[i][band]);

				in->nrelax[i] +=
				    nc * in->srh_n_r1[i][band] -
				    in->srh_n_r2[i][band];
				in->ntrap_to_p[i] +=
				    -(-pc * in->srh_n_r3[i][band] +
				      in->srh_n_r4[i][band]);

				in->nt_r1[i][band] = nc * in->srh_n_r1[i][band];
				in->nt_r2[i][band] = in->srh_n_r2[i][band];
				in->nt_r3[i][band] = pc * in->srh_n_r3[i][band];
				in->nt_r4[i][band] = in->srh_n_r4[i][band];

			}
		}
		//band=0;
		//printf("heren %le %le %le %le\n",in->Vapplied,nc*in->srh_n_r1[i][band]-in->srh_n_r2[i][band]-pc*in->srh_n_r3[i][band]+in->srh_n_r4[i][band],nc*in->srh_n_r1[i][band]-in->srh_n_r2[i][band],-pc*in->srh_n_r3[i][band]+in->srh_n_r4[i][band]);

		if (in->ptrapnewton == TRUE) {

			for (band = 0; band < in->srh_bands; band++) {
				//dJdtrapn[band]=0.0;
				dJpdtrapp[band] = 0.0;
				dJdtrapp[band] = 0.0;
				dptrap[band] =
				    pc * in->srh_p_r1[i][band] -
				    in->srh_p_r2[i][band] -
				    nc * in->srh_p_r3[i][band] +
				    in->srh_p_r4[i][band];
				dptrapdptrap[band] =
				    pc * in->dsrh_p_r1[i][band] -
				    in->dsrh_p_r2[i][band] -
				    nc * in->dsrh_p_r3[i][band] +
				    in->dsrh_p_r4[i][band];
				dptrapdp[band] = dpc * in->srh_p_r1[i][band];
				dptrapdn[band] = -dnc * in->srh_p_r3[i][band];

				Rtrapp +=
				    pc * in->srh_p_r1[i][band] -
				    in->srh_p_r2[i][band];
				dJpdxipc += dpc * in->srh_p_r1[i][band];
				dJpdtrapp[band] +=
				    pc * in->dsrh_p_r1[i][band] -
				    in->dsrh_p_r2[i][band];

				Rtrapn +=
				    -(-nc * in->srh_p_r3[i][band] +
				      in->srh_p_r4[i][band]);
				dJdxic -= -(-dnc * in->srh_p_r3[i][band]);
				dJdtrapp[band] -=
				    -(-nc * in->dsrh_p_r3[i][band] +
				      in->dsrh_p_r4[i][band]);

				in->prelax[i] +=
				    pc * in->srh_p_r1[i][band] -
				    in->srh_p_r2[i][band];
				in->ptrap_to_n[i] +=
				    -(-nc * in->srh_p_r3[i][band] +
				      in->srh_p_r4[i][band]);

				in->pt_r1[i][band] = pc * in->srh_p_r1[i][band];
				in->pt_r2[i][band] = in->srh_p_r2[i][band];
				in->pt_r3[i][band] = nc * in->srh_p_r3[i][band];
				in->pt_r4[i][band] = in->srh_p_r4[i][band];

			}

		}
		//band=0;
		//printf("hereh %le %le %le %le\n",in->Vapplied,pc*in->srh_p_r1[i][band]-in->srh_p_r2[i][band]-nc*in->srh_p_r3[i][band]+in->srh_p_r4[i][band],pc*in->srh_p_r1[i][band]-in->srh_p_r2[i][band],-nc*in->srh_p_r3[i][band]+in->srh_p_r4[i][band]);

		in->Rn[i] = Rtrapn;
		in->Rp[i] = Rtrapp;

		//Rtrapp=1e24;
		//Rtrapn=1e24;

		if (i != 0) {
			in->Ti[pos] = i;
			in->Tj[pos] = i - 1;
			in->Tx[pos] = dphil_d;
			pos++;
			//electron
			in->Ti[pos] = in->ymeshpoints * (1) + i;
			in->Tj[pos] = in->ymeshpoints * (1) + i - 1;
			in->Tx[pos] = dJdxil;
			//printf("a %le\n",in->Tx[pos]);
			pos++;

			in->Ti[pos] = in->ymeshpoints * (1) + i;
			in->Tj[pos] = i - 1;
			in->Tx[pos] = dJdphil;
			//printf("b %le\n",in->Tx[pos]);
			pos++;

			//hole
			in->Ti[pos] = in->ymeshpoints * (1 + 1) + i;
			in->Tj[pos] = in->ymeshpoints * (1 + 1) + i - 1;
			in->Tx[pos] = dJpdxipl;
			//printf("c %le\n",in->Tx[pos]);
			pos++;

			in->Ti[pos] = i + in->ymeshpoints * (1 + 1);
			in->Tj[pos] = i - 1;
			in->Tx[pos] = dJpdphil;
			//printf("d %le\n",in->Tx[pos]);
			pos++;

		}
		//getchar();
		if ((in->kl_in_newton == TRUE) && (in->interfaceleft == TRUE)
		    && (i == 0)) {
			//printf("%d\n",i);
			//getchar();
		} else {
			//phi
			in->Ti[pos] = i;
			in->Tj[pos] = i;
			in->Tx[pos] = dphic_d;
			pos++;
		}

		in->Ti[pos] = i;
		in->Tj[pos] = i + in->ymeshpoints * (1);
		in->Tx[pos] = dphidxic;
		//strcpy(in->Tdebug[pos],"dphidxic");
		pos++;

		in->Ti[pos] = i;
		in->Tj[pos] = i + in->ymeshpoints * (1 + 1);
		in->Tx[pos] = dphidxipc;
		//strcpy(in->Tdebug[pos],"dphidxipc");
		pos++;

		//electron

		in->Ti[pos] = in->ymeshpoints * (1) + i;
		in->Tj[pos] = in->ymeshpoints * (1) + i;
		in->Tx[pos] = dJdxic;
		//strcpy(in->Tdebug[pos],"dJdxic");
		pos++;

		in->Ti[pos] = in->ymeshpoints * (1) + i;
		in->Tj[pos] = in->ymeshpoints * (1 + 1) + i;
		in->Tx[pos] = dJdxipc;
		//strcpy(in->Tdebug[pos],"dJdxipc");
		pos++;

		in->Ti[pos] = in->ymeshpoints * (1) + i;
		in->Tj[pos] = i;
		in->Tx[pos] = dJdphic;
		//strcpy(in->Tdebug[pos],"dJdphic");
		pos++;

		//hole
		in->Ti[pos] = in->ymeshpoints * (1 + 1) + i;
		in->Tj[pos] = in->ymeshpoints * (1 + 1) + i;
		in->Tx[pos] = dJpdxipc;
		pos++;

		in->Ti[pos] = in->ymeshpoints * (1 + 1) + i;
		in->Tj[pos] = in->ymeshpoints * (1) + i;
		in->Tx[pos] = dJpdxic;
		pos++;

		in->Ti[pos] = in->ymeshpoints * (1 + 1) + i;
		in->Tj[pos] = i;
		in->Tx[pos] = dJpdphic;
		pos++;

		if (in->ntrapnewton == TRUE) {
			for (band = 0; band < in->srh_bands; band++) {
				in->Ti[pos] =
				    in->ymeshpoints * (1 + 1 + 1 + band) + i;
				in->Tj[pos] =
				    in->ymeshpoints * (1 + 1 + 1 + band) + i;
				in->Tx[pos] = dntrapdntrap[band];
				pos++;

				in->Ti[pos] =
				    in->ymeshpoints * (1 + 1 + 1 + band) + i;
				in->Tj[pos] = in->ymeshpoints * 1 + i;
				in->Tx[pos] = dntrapdn[band];
				pos++;

				in->Ti[pos] =
				    in->ymeshpoints * (1 + 1 + 1 + band) + i;
				in->Tj[pos] = in->ymeshpoints * (1 + 1) + i;
				in->Tx[pos] = dntrapdp[band];
				pos++;

				in->Ti[pos] = in->ymeshpoints * (1) + i;
				in->Tj[pos] =
				    in->ymeshpoints * (1 + 1 + 1 + band) + i;
				in->Tx[pos] = dJdtrapn[band];
				pos++;

				in->Ti[pos] = in->ymeshpoints * (1 + 1) + i;
				in->Tj[pos] =
				    in->ymeshpoints * (1 + 1 + 1 + band) + i;
				in->Tx[pos] = dJpdtrapn[band];
				pos++;

				in->Ti[pos] = i;
				in->Tj[pos] =
				    in->ymeshpoints * (1 + 1 + 1 + band) + i;
				in->Tx[pos] = dphidntrap[band];
				pos++;

			}

		}

		if (in->ptrapnewton == TRUE) {
			for (band = 0; band < in->srh_bands; band++) {
				in->Ti[pos] =
				    in->ymeshpoints * (1 + 1 + 1 +
						       in->srh_bands + band) +
				    i;
				in->Tj[pos] =
				    in->ymeshpoints * (1 + 1 + 1 +
						       in->srh_bands + band) +
				    i;
				in->Tx[pos] = dptrapdptrap[band];
				pos++;

				in->Ti[pos] =
				    in->ymeshpoints * (1 + 1 + 1 +
						       in->srh_bands + band) +
				    i;
				in->Tj[pos] = in->ymeshpoints * (1 + 1) + i;
				in->Tx[pos] = dptrapdp[band];
				pos++;

				in->Ti[pos] =
				    in->ymeshpoints * (1 + 1 + 1 +
						       in->srh_bands + band) +
				    i;
				in->Tj[pos] = in->ymeshpoints * (1) + i;
				in->Tx[pos] = dptrapdn[band];
				pos++;

				in->Ti[pos] = in->ymeshpoints * (1 + 1) + i;
				in->Tj[pos] =
				    in->ymeshpoints * (1 + 1 + 1 +
						       in->srh_bands + band) +
				    i;
				in->Tx[pos] = dJpdtrapp[band];
				pos++;

				in->Ti[pos] = in->ymeshpoints * (1) + i;
				in->Tj[pos] =
				    in->ymeshpoints * (1 + 1 + 1 +
						       in->srh_bands + band) +
				    i;
				in->Tx[pos] = dJdtrapp[band];
				pos++;

				in->Ti[pos] = i;
				in->Tj[pos] =
				    in->ymeshpoints * (1 + 1 + 1 +
						       in->srh_bands + band) +
				    i;
				in->Tx[pos] = dphidptrap[band];
				pos++;
			}

		}

		if (i != (in->ymeshpoints - 1)) {

			if ((in->kl_in_newton == TRUE)
			    && (in->interfaceleft == TRUE) && (i == 0)) {
				//printf("%d\n",i);
				//getchar();
			} else {
				//phi
				in->Ti[pos] = i;
				in->Tj[pos] = i + 1;
				in->Tx[pos] = dphir_d;
				pos++;
			}

			//electron
			in->Ti[pos] = in->ymeshpoints * (1) + i;
			in->Tj[pos] = in->ymeshpoints * (1) + i + 1;
			in->Tx[pos] = dJdxir;
			pos++;

			in->Ti[pos] = i + in->ymeshpoints * (1);
			in->Tj[pos] = i + 1;
			in->Tx[pos] = dJdphir;
			pos++;

			//hole
			in->Ti[pos] = in->ymeshpoints * (1 + 1) + i;
			in->Tj[pos] = in->ymeshpoints * (1 + 1) + i + 1;
			in->Tx[pos] = dJpdxipr;
			pos++;

			in->Ti[pos] = i + in->ymeshpoints * (1 + 1);
			in->Tj[pos] = i + 1;
			in->Tx[pos] = dJpdphir;
			pos++;

		}
		//Possion
		gdouble build = 0.0;
		if ((in->interfaceleft == TRUE) && (i == 0)) {
			build = -0.0;
		} else
		    if ((in->interfaceright == TRUE)
			&& (i == in->ymeshpoints - 1)) {
			build = -0.0;
		} else {
			build = -(deriv);

			build += -(-(pc - nc - Nad));

			for (band = 0; band < in->srh_bands; band++) {
				build +=
				    -(-Q * (in->pt[i][band] - in->nt[i][band]));
			}

			//build+= -(-Q*in->Nad[i]);
			//printf("n=%Le n0=%Le\n",in->n[i],in->n[i]*phi0);
			//printf("p=%Le p0=%Le\n",in->p[i],in->p[i]*phi0);
			//printf("Nad=%Le Nad0=%Le\n",in->Nad[i],in->Nad[i]*phi0);
			//printf("deriv_norm=%Le deriv=%Le\n",deriv*phi0,deriv);
		}
		in->b[i] = build;
		//getchar();
		//printf("%le\n",in->b[i]);
		//getchar();
		build = 0.0;
		build = -((Jnr - Jnl) / (dylh + dyrh) - Rtrapn - Rfree);

		//printf("p=%Le %Le %Le\n",pl,pc,pr);
		//printf("p=%Le %Le %Le\n",nl,nc,nr);
		//getchar();

		//printf("2 %le\n",in->b[in->ymeshpoints*(1)+i]);
		//getchar();

		//getchar();
		build -= Gn;
		in->b[in->ymeshpoints * (1) + i] = build;
		//printf("3 %Le %le\n",Gn,in->b[in->ymeshpoints*(1)+i]);
		//getchar();

		//hole
		build = 0.0;
		build = -((Jpr - Jpl) / (dylh + dyrh) + Rtrapp + Rfree);
		//printf("%le %le\n",Rtrapn,Rtrapp);

		build -= -Gp;

		in->b[in->ymeshpoints * (1 + 1) + i] = build;

		if (in->ntrapnewton == TRUE) {
			for (band = 0; band < in->srh_bands; band++) {
				in->b[in->ymeshpoints * (1 + 1 + 1 + band) +
				      i] = -(dntrap[band]);
			}
		}

		if (in->ptrapnewton == TRUE) {
			for (band = 0; band < in->srh_bands; band++) {
				in->b[in->ymeshpoints *
				      (1 + 1 + 1 + in->srh_bands + band) + i] =
				    -(dptrap[band]);
			}

		}

	}

	if (pos > in->N) {
		ewe(sim, "Error %d %d %d\n", pos, in->N, in->kl_in_newton);
	}
//solver_dump_matrix_comments(in->M,pos,in->Tdebug,in->Ti,in->Tj, in->Tx,in->b,"");
//getchar();
//fclose(file_j);
//printf("Check J file\n");
//getchar();

}

gdouble get_cur_error(struct simulation *sim, struct device *in)
{
	int i;
	gdouble phi = 0.0;
	gdouble n = 0.0;
	gdouble p = 0.0;
	gdouble x = 0.0;
	gdouble te = 0.0;
	gdouble th = 0.0;
	gdouble tl = 0.0;
	gdouble ttn = 0.0;
	gdouble ttp = 0.0;
	gdouble i0 = 0.0;
	int band = 0;
	for (i = 0; i < in->ymeshpoints; i++) {
		if ((in->interfaceleft == TRUE) && (i == 0)) {
		} else
		    if ((in->interfaceright == TRUE)
			&& (i == in->ymeshpoints - 1)) {
		} else {
			phi += gfabs(in->b[i]);
			//printf("%Le\n",in->b[i]);
		}

		n += gfabs(in->b[in->ymeshpoints * (1) + i]);
		p += +gfabs(in->b[in->ymeshpoints * (1 + 1) + i]);

		if (in->ntrapnewton == TRUE) {
			for (band = 0; band < in->srh_bands; band++) {
				ttn +=
				    gfabs(in->
					  b[in->ymeshpoints *
					    (1 + 1 + 1 + band) + i]);
				//printf("error %le\n",fabs(in->b[in->ymeshpoints*(1+1+1+band)+i]));
			}
		}

		if (in->ptrapnewton == TRUE) {
			for (band = 0; band < in->srh_bands; band++) {
				ttp +=
				    gfabs(in->
					  b[in->ymeshpoints *
					    (1 + 1 + 1 + in->srh_bands + band) +
					    i]);
			}
		}

	}
	gdouble tot = phi + n + p + x + te + th + tl + ttn + ttp + i0;
//printf("%Le %Le %Le %Le %Le %Le %Le %Le %Le\n",phi,n,p,x,te,th,tl,ttn,ttp);
	if (isnan(tot)) {
		printf("%Le %Le %Le %Le %Le %Le %Le %Le %Le\n", phi, n, p, x,
		       te, th, tl, ttn, ttp);
		(*fun->dump_matrix) (in->M, in->N, in->Ti, in->Tj, in->Tx,
				     in->b, "");

		ewe(sim, "nan detected in newton solver\n");
	}

	return tot;
}

gdouble get_abs_error(struct device * in)
{
	int i;
	gdouble tot = 0.0;

	for (i = 0; i < in->M; i++) {
		tot += gfabs(in->b[i]);
	}

	return tot;
}

void solver_cal_memory(struct device *in, int *ret_N, int *ret_M)
{
	int i = 0;
	int N = 0;
	int M = 0;
	N = in->ymeshpoints * 3 - 2;	//Possion main

	N += in->ymeshpoints * 3 - 2;	//Je main
	N += in->ymeshpoints * 3 - 2;	//Jh main

	N += in->ymeshpoints * 3 - 2;	//dJe/phi
	N += in->ymeshpoints * 3 - 2;	//dJh/phi

	N += in->ymeshpoints;	//dphi/dn
	N += in->ymeshpoints;	//dphi/dh

	N += in->ymeshpoints;	//dJndp
	N += in->ymeshpoints;	//dJpdn

	M = in->ymeshpoints;	//Pos

	M += in->ymeshpoints;	//Je
	M += in->ymeshpoints;	//Jh

	if (in->ntrapnewton == TRUE) {
		for (i = 0; i < in->srh_bands; i++) {
			N += in->ymeshpoints;	//dntrapdn
			N += in->ymeshpoints;	//dntrapdntrap
			N += in->ymeshpoints;	//dntrapdp
			N += in->ymeshpoints;	//dJndtrapn
			N += in->ymeshpoints;	//dJpdtrapn
			N += in->ymeshpoints;	//dphidntrap

			M += in->ymeshpoints;	//nt
		}

	}

	if (in->ptrapnewton == TRUE) {
		for (i = 0; i < in->srh_bands; i++) {
			N += in->ymeshpoints;	//dptrapdp
			N += in->ymeshpoints;	//dptrapdptrap
			N += in->ymeshpoints;	//dptrapdn
			N += in->ymeshpoints;	//dJpdtrapp
			N += in->ymeshpoints;	//dJdtrapp
			N += in->ymeshpoints;	//dphidptrap

			M += in->ymeshpoints;	//pt
		}
	}
	*ret_N = N;
	*ret_M = M;
}

void dllinternal_solver_free_memory(struct device *in)
{
	if (in->srh_bands > 0) {
		free(dntrap);
		free(dntrapdntrap);
		free(dntrapdn);
		free(dntrapdp);
		free(dJdtrapn);
		free(dJpdtrapn);
		free(dphidntrap);
		free(dptrapdp);
		free(dptrapdptrap);
		free(dptrap);
		free(dptrapdn);
		free(dJpdtrapp);
		free(dJdtrapp);
		free(dphidptrap);

	}

	free(in->Ti);
	free(in->Tj);
	free(in->Tx);
	free(in->b);

//for (i=0;i<in->N;i++)
//{
//      free(in->Tdebug[i]);
//}
//free(in->Tdebug);

	dntrapdntrap = NULL;
	dntrap = NULL;
	dntrapdn = NULL;
	dntrapdp = NULL;
	dJdtrapn = NULL;
	dJpdtrapn = NULL;
	dphidntrap = NULL;
	dptrapdp = NULL;
	dptrapdptrap = NULL;
	dptrap = NULL;
	dptrapdn = NULL;
	dJpdtrapp = NULL;
	dJdtrapp = NULL;
	dphidptrap = NULL;

	in->Ti = NULL;
	in->Tj = NULL;
	in->Tx = NULL;
	in->b = NULL;
	in->Tdebug = NULL;

}

int dllinternal_solve_cur(struct simulation *sim, struct device *in)
{
	gdouble last_J = get_J(in);
	gdouble delta_J = 0.0;

	gdouble error = 0.0;
	int ittr = 0;
	if (get_dump_status(sim, dump_print_newtonerror) == TRUE) {
		printf("Solve cur\n");
	}

#ifdef only
	only_update_thermal = FALSE;
#endif
//in->enable_back=FALSE;
	int stop = FALSE;
	int thermalrun = 0;
	gdouble check[10];
	int cpos = 0;
//for (i=0;i<in->ymeshpoints;i++)
//{
//      printf("Rod ------- nt= %d %le\n",i,in->Gn[i]);
//}
	gdouble abs_error = 0.0;
	int e0 = 0;
	do {

		fill_matrix(sim, in);
		abs_error = get_abs_error(in);

//dump_for_plot(in);
//plot_now(in,"plot");
		//solver_dump_matrix(in->M,in->N,in->Ti,in->Tj, in->Tx,in->b);
//getchar();

		if (in->stop == TRUE) {
			break;
		}

		(*fun->solver) (in->M, in->N, in->Ti, in->Tj, in->Tx, in->b);

		update_solver_vars(sim, in, TRUE);
		//printf("Going to clamp=%d\n",proper);
		//solver_dump_matrix(in->M,in->N,in->Ti,in->Tj, in->Tx,in->b);
		//printf("%d\n");
		//getchar();    
		delta_J = fabs(last_J - get_J(in));
		last_J = get_J(in);
		error = get_cur_error(sim, in);

		//thermalrun++;
		if (thermalrun == 40)
			thermalrun = 0;
		//update(in);
//getchar();

		if (get_dump_status(sim, dump_print_newtonerror) == TRUE) {
			printf
			    ("%d error = %Le  abs_error = %Le dJ=%Le %Le I=%Le",
			     ittr, error, abs_error, delta_J, in->Vapplied,
			     get_J(in));

			printf("\n");
		}
		error = delta_J;
		in->last_error = error;
		in->last_ittr = ittr;
		ittr++;

		if (get_dump_status(sim, dump_write_converge) == TRUE) {
			sim->converge =
			    fopena(get_output_path(sim), "converge.dat", "a");
			fprintf(sim->converge, "%Le\n", error);
			fclose(sim->converge);
		}

		stop = TRUE;

		if (ittr < in->max_electrical_itt) {
			if (error > in->min_cur_error) {
				stop = FALSE;
				e0 = 0;
			} else {
				if (e0 < 10)
					stop = FALSE;
				e0++;
			}
		}

		if (ittr < in->newton_min_itt) {
			stop = FALSE;
		}

		if (in->newton_clever_exit == TRUE) {
			check[cpos] = error;
			cpos++;

			if (cpos > 10) {
				cpos = 0;
			}

			if (ittr >= in->newton_min_itt) {
				if ((check[0] < error) || (check[1] < error)) {
					stop = TRUE;
				}
			}
		}

	} while (stop == FALSE);

	in->newton_last_ittr = ittr;

	if (error > 1e-3) {
		printf_log(sim,
			   "warning: The solver has not converged very well.\n");
	}
//getchar();
	if (get_dump_status(sim, dump_newton) == TRUE) {
		(*fun->dump_1d_slice) (sim, in, get_output_path(sim));
	}
//plot_now(in,"plot");
//getchar();
	in->odes += in->M;
//getchar();

	return 0;
}

void dllinternal_solver_realloc(struct simulation *sim, struct device *in)
{
	int N = 0;
	int M = 0;
	long double *dtemp = NULL;
	int *itemp = NULL;

	solver_cal_memory(in, &N, &M);

	int alloc = FALSE;
	if ((in->N == 0) || (in->M == 0)) {
		in->N = N;
		in->M = M;
		alloc = TRUE;
	} else if ((N != in->N) || (M != in->M)) {
		in->N = N;
		in->M = M;
		alloc = TRUE;
	}

	if (alloc == TRUE) {

		itemp = realloc(in->Ti, in->N * sizeof(int));
		if (itemp == NULL) {
			ewe(sim, "memory error\n");
		} else {
			in->Ti = itemp;
		}

		itemp = realloc(in->Tj, in->N * sizeof(int));
		if (itemp == NULL) {
			ewe(sim, "memory error\n");
		} else {
			in->Tj = itemp;
		}

		dtemp = realloc(in->Tx, in->N * sizeof(long double));
		if (dtemp == NULL) {
			ewe(sim, "memory error\n");
		} else {
			in->Tx = dtemp;
		}

		//int i=0;
		//in->Tdebug = (char**)malloc(in->N*sizeof(char*));

		//for (i=0;i<in->N;i++)
		//{
		//      in->Tdebug[i]= (char*)malloc(20*sizeof(char));
		//      strcpy(in->Tdebug[i],"");
		//}

		dtemp = realloc(in->b, in->M * sizeof(long double));
		if (dtemp == NULL) {
			ewe(sim, "memory error\n");
		} else {
			in->b = dtemp;
		}

		if (in->srh_bands > 0) {
			dtemp =
			    realloc(dntrap, in->srh_bands * sizeof(gdouble));
			if (dtemp == NULL) {
				ewe(sim, "memory error\n");
			} else {
				dntrap = dtemp;
			}

			dtemp =
			    realloc(dntrapdntrap,
				    in->srh_bands * sizeof(gdouble));
			if (dtemp == NULL) {
				ewe(sim, "memory error\n");
			} else {
				dntrapdntrap = dtemp;
			}

			dtemp =
			    realloc(dntrapdn, in->srh_bands * sizeof(gdouble));
			if (dtemp == NULL) {
				ewe(sim, "memory error\n");
			} else {
				dntrapdn = dtemp;
			}

			dtemp =
			    realloc(dntrapdp, in->srh_bands * sizeof(gdouble));
			if (dtemp == NULL) {
				ewe(sim, "memory error\n");
			} else {
				dntrapdp = dtemp;
			}

			dtemp =
			    realloc(dJdtrapn, in->srh_bands * sizeof(gdouble));
			if (dtemp == NULL) {
				ewe(sim, "memory error\n");
			} else {
				dJdtrapn = dtemp;
			}

			dtemp =
			    realloc(dJpdtrapn, in->srh_bands * sizeof(gdouble));
			if (dtemp == NULL) {
				ewe(sim, "memory error\n");
			} else {
				dJpdtrapn = dtemp;
			}

			dtemp =
			    realloc(dphidntrap,
				    in->srh_bands * sizeof(gdouble));
			if (dtemp == NULL) {
				ewe(sim, "memory error\n");
			} else {
				dphidntrap = dtemp;
			}

			dtemp =
			    realloc(dptrapdp, in->srh_bands * sizeof(gdouble));
			if (dtemp == NULL) {
				ewe(sim, "memory error\n");
			} else {
				dptrapdp = dtemp;
			}

			dtemp =
			    realloc(dptrapdptrap,
				    in->srh_bands * sizeof(gdouble));
			if (dtemp == NULL) {
				ewe(sim, "memory error\n");
			} else {
				dptrapdptrap = dtemp;
			}

			dtemp =
			    realloc(dptrap, in->srh_bands * sizeof(gdouble));
			if (dtemp == NULL) {
				ewe(sim, "memory error\n");
			} else {
				dptrap = dtemp;
			}

			dtemp =
			    realloc(dptrapdn, in->srh_bands * sizeof(gdouble));
			if (dtemp == NULL) {
				ewe(sim, "memory error\n");
			} else {
				dptrapdn = dtemp;
			}

			dtemp =
			    realloc(dJpdtrapp, in->srh_bands * sizeof(gdouble));
			if (dtemp == NULL) {
				ewe(sim, "memory error\n");
			} else {
				dJpdtrapp = dtemp;
			}

			dtemp =
			    realloc(dJdtrapp, in->srh_bands * sizeof(gdouble));
			if (dtemp == NULL) {
				ewe(sim, "memory error\n");
			} else {
				dJdtrapp = dtemp;
			}

			dtemp =
			    realloc(dphidptrap,
				    in->srh_bands * sizeof(gdouble));
			if (dtemp == NULL) {
				ewe(sim, "memory error\n");
			} else {
				dphidptrap = dtemp;
			}

		}

	}

}
