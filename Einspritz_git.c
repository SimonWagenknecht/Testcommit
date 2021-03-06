
// Pull request
// 2. Commit
//1. Commit
//// -------------------- Funktion: Regelung des Einspritzventils der WP (1s-Takt) ------------
//// 16.06.2016_SiWa

#include <string.h>
#include "struct.h"
#include "ustruct.h"
#include "defins.h"
#include "sramext.h"
#include "uramext.h"
#include "standdef.h"
#include "unoinitext.h"
#include "uconstext.h"
#include "Int_Regeldifferenz.h"



///*---------------------- Einspritz_RV -----------------------------------*/
//// Takt: 1 s

void Einspritz_RV(void)

	{
	
		
	//float int_RW;         // integrierte Regelabweichung						
		
	float fl_ei = 0;  		// Regelabweichung
	float fl_dy_rel = 0;	// Berechnung der relativen Stellgröße
	
		// SiWa 
	static char neustart_SiWa = 1;		// Programmstart
	static char sch_current_BM = 0;
	static char sch_recent_BM = 0; 
	
	//Testen
	//DM_ANF_Ext1.messw=500;
	//DM_ANF_Ext1.stat=0;
	
			// Sollwert berechnen
			swd[0].Sollwert_berechnet =hkd[HK1].tvsb + sws[0].Sollwert_offset;
	
			swd[0].int_RW = (Int_Regelabweichung(swd[0].Sollwert_berechnet,TP_UNI[U5]->messw))/60;
			swd[0].int_RW_Anz	= (int) (swd[0].int_RW * 100); // Für Anzeige mit 2 Nachkomma
	
		//Handbetrieb des Einspritzventils
		
		 if (sws[0].ERV_Haut == 1) // Handbetrieb aktiv
			{
				AA_UNI[U6]->awert = sws[0].ERVstell_Hand;  // Regelventil übernimmt den Wert vom Handbetrieb
			}
			
		else 
				
			{	
		// Automatische Steuerung
	  	// Steuerung und Regelung des Einspritzventils für Fernwärme  
	  		// Ist der Fühler nicht in Ordnung, steht das Einspritzventil auf 50%
	  	  
	  	  				 	if (TP_UNI[U5]->stat!= 0 )
								 		{
				 							AA_UNI[U6]->awert = 500; 
				 							swd[0].SekBedarf_Freigabe = 1; // Bedarf
				 							
										}	
													
	  
	  							else
	  							// Bestimmung des Betriebszustands "Fernwärmebedarf"
										
	 									{
	 										if (hkd[HK1].tvsb == 0 || (hkd[HK1].tvsb > 100 && (TP_UNI[U5]->messw>sws[0].t_Sek_off+swd[0].Sollwert_berechnet || swd[0].int_RW > sws[0].K_dt_Sek_off/10))	)
	 										{
	 											swd[0].SekBedarf_Freigabe = 0; // kein Bedarf
	 										}
	 										else if (hkd[HK1].tvsb > 100 && (TP_UNI[U5]->messw<swd[0].Sollwert_berechnet-sws[0].t_Sek_on || swd[0].int_RW < sws[0].K_dt_Sek_on/10))	
	 											{
	 												swd[0].SekBedarf_Freigabe = 1; // Bedarf
	 											}
	 									}
	 									// Regelung des Ventils RV WE
	 										 	// Bei Aktivierung der Regelung die Ausgangsstellung des Ventils auf 100% setzen
													if (neustart_SiWa == 0)
															{
																sch_current_BM = swd[0].SekBedarf_Freigabe;  
																
															if (sch_recent_BM == 0 && sch_current_BM ==0)  // negative Flanke
																	{
																		sch_recent_BM = 1;
																	}	
															else if (sch_recent_BM == 1 && sch_current_BM ==1) // positive Flanke
																				{
																				sch_recent_BM = 0;
																						swd[0].EV_fl_y_rel = 100;
																				}
															}	
	 														if (neustart_SiWa == 1)
																{
																	neustart_SiWa = 0;
																}	
	 									
	 									if (swd[0].SekBedarf_Freigabe > 0)
	 										{
	 												
	
	 												
	 												
	 												//----------------------------------------------------------------------
													//	Reglertyp: P- / PID-Regler
													//	----------------------------------------------------------------------
																												
													// Regelabweichung = Soll - Ist
																	fl_ei	= (float)swd[0].Sollwert_berechnet - (float)TP_UNI[U5]->messw;
															
		                	
													// -------------- PID-Regelalgorithmus ---------------------------------
													// Berechnung der relativen Stellgrößen
						  						fl_dy_rel = Dy_rel ( sws[0].EV_Kp, sws[0].EV_Kd, sws[0].EV_Ts, sws[0].EV_Tn, fl_ei, swd[0].EV_fl_ei1, swd[0].EV_fl_ei2 );
                    			
									  			swd[0].EV_fl_y_rel -= fl_dy_rel;   // 0 Volt = Ist < Soll, 10 Volt = Ist > Soll, Berechnung der absoluten Stellgröße wird aus relative Stellfröße gebildet
		                			
													// Regelabweichungen merken
													swd[0].EV_fl_ei2 = swd[0].EV_fl_ei1;
													swd[0].EV_fl_ei1 = fl_ei;						
													// Parameterumwandlung float to int
													swd[0].EV_ei = (int)(fl_ei);											// [K] zur Anzeige
                    			
													// Berechnung und Ausgabe der absoluten Stellgrößen
													// --------------------------------------------------------------------			
													// Ausgabe an 0-10V Ventil   
						        			
													//			fl_Y_rel_beg = (float)pHks->Y_rel_beg / 10;			// Beginn der Ventilöffnung
													//			fl_Y_rel_min = (float)pHks->Y_rel_min / 10;			// Minimalbegrenzung
													//			
														if(swd[0].EV_fl_y_rel  > sws[0].EV_max)								// Begrenzung max 100 %
															 swd[0].EV_fl_y_rel  = sws[0].EV_max;
													//			
													//			if(fl_dy_rel > 1)																// nur bei positiver Änderung
													//			{	if(pHkd->fl_y_rel < fl_Y_rel_beg)							// und Wert kleiner als Ventilöffnungsbeginn,
													//					 pHkd->fl_y_rel = fl_Y_rel_beg;							// dann Setzen auf Ventilöffnungsbeginn
													//			}
													//			
														if(swd[0].EV_fl_y_rel  < sws[0].EV_min)								
															 swd[0].EV_fl_y_rel  = sws[0].EV_min;							// Minimalbegrenzung
								    			
								    			
				            			
															swd[0].EV_y_rel = (int)(swd[0].EV_fl_y_rel * 10);				// Zur Anzeige und Weiterverarbeitung
															AA_UNI[U6]->awert = swd[0].EV_y_rel;									// Ausgabe an Stellventil
	 										}
	 										else
	 											
	 											{
	 											AA_UNI[U6]->awert = 1000;
	 											}
	 			}

	}

