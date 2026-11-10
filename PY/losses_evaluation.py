import numpy as np
import matplotlib.pyplot as plt

# =============================================================================
# 1. PARAMETRI DI SISTEMA E COMPONENTI (FASE 1 - PROGETTO TERMICO)
# =============================================================================
f_sw = 400e3          # Frequenza di switching [Hz]
I_in = 8.0            # Corrente media induttore / ingresso per leaf [A]
V_in_nom = 12.0       # Tensione ingresso nominale [V]
V_out_nom = 44.4      # Tensione uscita nominale LiPo 12S [V]
V_out_max = 50.4      # Tensione uscita massima LiPo 12S [V]
T_amb_max = 60.0      # Temperatura ambiente max nel vano [°C] (30°C + 30°C)

# MOSFET: Infineon BSC110N15NS5 (SuperSO8)
R_dson_25C = 11.0e-3  # [Ohm] max a 25°C
alpha_mos = 0.004     # Coefficiente termico R_dson [1/K]
t_on = 10e-9          # Tempo di turn-on [s] (da tesi Sez. 4.4.3.1)
t_off = 10e-9         # Tempo di turn-off [s]
C_oss_er = 500e-12    # [ASSUNZIONE] Capacità di uscita equivalente energetica [F]
R_thJC_mos_bot = 1.0  # Resistenza termica Giunzione-Pad inferiore [K/W]
R_thJC_mos_top = 18.0 # [ASSUNZIONE] Resistenza termica Giunzione-Top plastico [K/W]

# Diodo Boost: Vishay V60D100C (TO-263AB / D2PAK)
V_F_boost = 0.66      # Caduta diretta a ~2-4 A, Tj=100°C [V] (da tesi Sez. 4.4.3.2)
R_d_boost = 15e-3     # Resistenza differenziale diodo [Ohm]
Q_c_boost = 15e-9     # [ASSUNZIONE] Carica capacitiva Schottky a 50V [C]
R_thJC_diode_bot = 0.8# Resistenza termica Giunzione-Pad catodo [K/W]
R_thJC_diode_top = 12.0 # [ASSUNZIONE] Resistenza Giunzione-Top plastico D2PAK [K/W]

# Altri componenti della singola Leaf
C_snub = 3 * 550e-12  # Capacità snubber (~1.65 nF) [F]
V_F_rev = 0.50        # [ASSUNZIONE] Caduta diretta SK2040YD2R a 8 A [V]
P_ind_nolitz = 1.009 + 0.15 # Perdite Cu (Tab. 4.4 tesi) + Core [W]
P_ind_litz = 0.343 + 0.15   # Perdite Cu Litz (Tab. 4.4 tesi) + Core [W]
P_driver = 0.18       # Perdite Gate Driver IR2110S + Rg [W]
P_sensor = 0.13       # Perdite ACS712 [W]

# =============================================================================
# 2. CALCOLO CORRENTI E PERDITE AL VARIARE DEL DUTY CYCLE (0.4 <= D <= 0.85)
# =============================================================================
D_vec = np.linspace(0.40, 0.85, 100)
delta_I = 0.8         # Ripple corrente induttore picco-picco [A]

# Correnti corrette (verifica contro Fig. 4.18 della tesi)
I_L_rms = np.sqrt(I_in**2 + (delta_I**2) / 12.0)
I_sw_rms = I_L_rms * np.sqrt(D_vec)
I_d_avg = I_in * (1.0 - D_vec)
I_d_rms = I_L_rms * np.sqrt(1.0 - D_vec)

# Calcolo perdite in funzione di D (fissando V_out = V_out_max per worst-case tensione)
T_j_est = 105.0 # Temperatura di riferimento per calcolo R_dson
R_dson_hot = R_dson_25C * 2
P_mos_cond = (I_sw_rms**2) * R_dson_hot
P_mos_sw = 0.5 * V_out_max * I_in * (t_on + t_off) * f_sw + 0.5 * C_oss_er * (V_out_max**2) * f_sw
P_mos_tot = P_mos_cond + P_mos_sw

P_diode_cond = V_F_boost * I_d_avg + (I_d_rms**2) * R_d_boost
P_diode_sw = V_out_max * Q_c_boost * f_sw
P_diode_tot = P_diode_cond + P_diode_sw

P_snub = C_snub * (V_out_max**2) * f_sw
P_rev_diode = V_F_rev * I_in

# =============================================================================
# 3. ANALISI DEL PUNTO NOMINALE E WORST-CASE
# =============================================================================
D_nom = 1.0 - (V_in_nom / V_out_nom) # 0.730
D_wc  = 1.0 - (10.0 / V_out_max)     # 0.802

def calc_point_losses(D, V_out):
    i_sw_rms = I_L_rms * np.sqrt(D)
    i_d_avg = I_in * (1.0 - D)
    i_d_rms = I_L_rms * np.sqrt(1.0 - D)
    p_m_c = (i_sw_rms**2) * R_dson_hot
    p_m_s = 0.5 * V_out * I_in * (t_on + t_off) * f_sw + 0.5 * C_oss_er * (V_out**2) * f_sw
    p_d_c = V_F_boost * i_d_avg + (i_d_rms**2) * R_d_boost
    p_d_s = V_out * Q_c_boost * f_sw
    p_sn  = C_snub * (V_out**2) * f_sw
    return {
        'I_sw_rms': i_sw_rms, 'I_d_avg': i_d_avg,
        'P_mos': p_m_c + p_m_s, 'P_mos_cond': p_m_c, 'P_mos_sw': p_m_s,
        'P_diode': p_d_c + p_d_s, 'P_diode_cond': p_d_c, 'P_diode_sw': p_d_s, 'P_snub': p_sn,
        'P_rev': P_rev_diode, 'P_ind': P_ind_nolitz,
        'P_aux': P_driver + P_sensor
    }

nom = calc_point_losses(D_nom, V_out_nom)
wc  = calc_point_losses(D_wc, V_out_max)

print("=== RIEPILOGO PERDITE PER LEAF ===")
for label, res in [("NOMINALE (12V->44.4V, D=0.73)", nom), ("WORST-CASE (10V->50.4V, D=0.80)", wc)]:
    tot = res['P_mos'] + res['P_diode'] + res['P_snub'] + res['P_rev'] + res['P_ind'] + res['P_aux']
    print(f"\n--- {label} ---")
    print(f"I_sw,rms = {res['I_sw_rms']:.2f} A | I_D,avg = {res['I_d_avg']:.2f} A")
    print(f"MOSFET Totale : {res['P_mos']:.2f} W (Cond: {res['P_mos_cond']:.2f} W, Sw: {res['P_mos_sw']:.2f} W)")
    print(f"Diodo Boost   : {res['P_diode']:.2f} W (Cond: {res['P_diode_cond']:.2f} W, Sw: {res['P_diode_sw']:.2f} W)")
    print(f"Snubber RC    : {res['P_snub']:.2f} W")
    print(f"Diodo Ingr.   : {res['P_rev']:.2f} W")
    print(f"Induttore     : {res['P_ind']:.2f} W")
    print(f"Driver+Sens.  : {res['P_aux']:.2f} W")
    print(f"TOTALE LEAF   : {tot:.2f} W")

# =============================================================================
# 4. RETE TERMICA E GRAFICI DI PROGETTO
# =============================================================================
fig, axs = plt.subplots(1, 3, figsize=(16, 4.8))

# Plot 1: Correnti corrette vs Duty Cycle (Sostituisce Fig. 4.18 della tesi)
axs[0].plot(D_vec, I_sw_rms, 'b-', lw=2, label=r'$I_{\mathrm{SW,rms}}$ [A]')
axs[0].plot(D_vec, I_d_avg, 'r--', lw=2, label=r'$I_{\mathrm{D,avg}}$ [A]')
axs[0].axvline(D_nom, color='gray', linestyle=':', label=f'$D_{{nom}}={D_nom:.2f}$')
axs[0].set_title('Correnti su MOSFET e Diodo ($I_{in}=8$ A)')
axs[0].set_xlabel('Duty Cycle $D$')
axs[0].set_ylabel('Corrente [A]')
axs[0].grid(True, alpha=0.4)
axs[0].legend()

# Plot 2: Potenze dissipate nei semiconduttori vs Duty Cycle
axs[1].plot(D_vec, P_mos_tot, 'b-', lw=2, label='MOSFET Totale ($P_{MOS}$)')
axs[1].plot(D_vec, P_mos_cond, 'b:', lw=1.5, label='MOSFET Conduzione')
axs[1].plot(D_vec, P_diode_tot, 'r-', lw=2, label='Diodo Boost ($P_{D}$)')
axs[1].plot(D_vec, np.full_like(D_vec, P_snub), 'g-.', lw=1.5, label='Snubber RC ($P_{snub}$)')
axs[1].set_title(f'Perdite Semiconduttori e Snubber ($V_{{out}}={V_out_max}$ V)')
axs[1].set_xlabel('Duty Cycle $D$')
axs[1].set_ylabel('Potenza Dissipata [W]')
axs[1].grid(True, alpha=0.4)
axs[1].legend()

# Plot 3: Temperatura di giunzione MOSFET vs R_thJA,eff per T_amb = 30°C e 60°C
R_th_vec = np.linspace(5, 35, 100)
Tj_60 = 60.0 + wc['P_mos'] * R_th_vec
Tj_30 = 30.0 + wc['P_mos'] * R_th_vec

axs[2].plot(R_th_vec, Tj_60, 'r-', lw=2, label=r'$T_a = 60\,^\circ\mathrm{C}$ (Max Vano)')
axs[2].plot(R_th_vec, Tj_30, 'b--', lw=2, label=r'$T_a = 30\,^\circ\mathrm{C}$ (Esterno)')
axs[2].axhline(150, color='k', linestyle='-', lw=1.5, label=r'$T_{j,\max} = 150\,^\circ\mathrm{C}$ (Limite fisico)')
axs[2].axhline(115, color='orange', linestyle='--', lw=1.5, label=r'$T_{j,\mathrm{derated}} = 115\,^\circ\mathrm{C}$ (IPC-9592B)')
axs[2].axhline(100, color='green', linestyle=':', lw=1.5, label=r'$T_{j,\mathrm{target}} = 100\,^\circ\mathrm{C}$')
axs[2].set_title(f'Temp. Giunzione MOSFET ($P_{{MOS,wc}}={wc["P_mos"]:.2f}$ W)')
axs[2].set_xlabel(r'Resistenza Termica Equivalente $R_{\theta JA,\mathrm{eff}}$ [K/W]')
axs[2].set_ylabel(r'Temperatura di Giunzione $T_j$ [°C]')
axs[2].set_ylim(40, 165)
axs[2].grid(True, alpha=0.4)
axs[2].legend(fontsize=8)

plt.tight_layout()
plt.savefig('thermal_analysis_phase1.png', dpi=300)
plt.show()