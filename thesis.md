## **POLITECNICO DI TORINO** 

**Corso di Laurea Magistrale in Ingegneria Elettronica** 



<!-- Start of picture text -->
v & yy<br>$4.44 Politecnico<br>(meme di Torino<br>Nal<br><!-- End of picture text -->



<!-- Start of picture text -->
SE ID<br>bg) UNIVERSITAT<br>Wl, POLITECNICA<br>CTOs DE VALENCIA<br><!-- End of picture text -->

**Four-Phases Interleaved Boost Converter for an UAV Hydrogen Fuel Cell System** 

**Supervisors Candidate Dr. Erica RAVIOLA Davide CATANIA Dr. Carlos SÁNCHEZ DÍAZ** 

**DECEMBER 2026** 

### **Abstract** 

To address the challenge of sustainable transportation, alternative energy sources, such as the Hydrogen Fuel Cells (HFCs), have been developed in recent years by the automotive industry. Their adoption in the Unmanned Aerial Vehicle (UAV) agricultural sector is determined by the tradeoff between the flight time and payload (fertilizer or seeds), which is strictly related on the energy storage capacity. However, HFCs cannot be used directly as a power supply due to the large current fluctuations in drone motors during operation. Consequently, an additional stack of fast discharge batteries, like lithium-polymer (LiPo), must be introduced within the system. Typically, to meet the requirements of both UAV actuators and HFCs, a conventional boost topology is employed. However, to minimize the current ripple at the input, large inductance values are required, resulting in a heavier power stage. 

Within the LIGH2TDRONE project activities carried out at the Universitat Politècnica de València, this thesis focuses on the analysis, design and implementation of a Four-Phases Interleaved Boost Converter (IBC) with digital control. The generic N-phase structure is analyzed to assess the advantages of the interleaved topology, including the reduction in size of the magnetic components and output filter, which are typically bulky and heavy, thereby decreasing the power stage weight. Additionally, the ability to connect multiple inputs thanks to its inherent modularity, enhancing overall system reliability, output current capability as well as the limitation of the input current ripple to prevent harmful phenomena inside the cell, such as fuel starvation. 

A theoretical overview of the Interleaved topology, including its small-signal model, is performed with the consequent analysis of the effects on the components sizes. Simulations with varying number of legs have been performed, with the objective of identifying the optimal number of phases, by looking at different parameters as input current ripple, output voltage ripple, and converter efficiency. After evaluating all component stresses for the worst-case scenario, commercially available devices are selected. Consequently, a 33 _µ_ H custom inductor is designed and assembled using the Area-Product method, starting from the assessment of the core constraints in terms of core saturation and maximum temperature rise, followed by the selection of a commercial core model and wire gauge, alongside theoretical loss evaluation and laboratory characterization. Different options to maximize the resolution of the conditioning circuitry are designed and compared in terms of overall component count and adjustment feasibility to be compatible with the STM32 ADC frontend. In addition, both the analog and digital compensator design procedure is presented along with the related frequency response to assess stability criteria and performance. Finally, a four-layer printed circuit board is manufactured and assembled following the provided specifications, accompanied by a cost breakdown and an overview of potential modifications and future developments. 

II 

_Alla mia famiglia_ 

_"Farsi primavera, significa accettare il rischio dell’inverno. Farsi presenza, significa accettare il rischio dell’assenza"_ 

- Antoine de Saint-Exupéry 

# **Acknowledgements** 

IV 

# **Table of Contents** 

|**1**|**Intr**|**oduction**<br>**1**|
|---|---|---|
||1.1|Hydrogen in transportation . . . . . . . . . . . . . . . . . . . . . . .<br>2|
||1.2|Power Converters in Fuel Cell Applications<br>. . . . . . . . . . . . . .<br>4|
|||1.2.1<br>The Interleaved configuration . . . . . . . . . . . . . . . . . .<br>4|
||1.3|Scope of the Thesis . . . . . . . . . . . . . . . . . . . . . . . . . . . .<br>5|
|||1.3.1<br>Project Specifications<br>. . . . . . . . . . . . . . . . . . . . . .<br>6|
||1.4|Thesis Outline<br>. . . . . . . . . . . . . . . . . . . . . . . . . . . . . .<br>7|
|**2**|**Fue**|**l Cell Systems Overview**<br>**8**|
||2.1|PEMFC Architecture and Reactions . . . . . . . . . . . . . . . . . .<br>9|
||2.2|PEMFC Equivalent Circuit Model<br>. . . . . . . . . . . . . . . . . . .<br>10|
|**3**|**Inte**|**rleaved Boost Converter Analysis**<br>**12**|
||3.1|Conventional Boost Converter . . . . . . . . . . . . . . . . . . . . . .<br>12|
|||3.1.1<br>Steady-State Analysis . . . . . . . . . . . . . . . . . . . . . .<br>13|
|||3.1.2<br>Inductor Analysis . . . . . . . . . . . . . . . . . . . . . . . . .<br>15|
|||3.1.2.1<br>CCM Operation . . . . . . . . . . . . . . . . . . . .<br>16|
|||3.1.3<br>Output Capacitor _CO_<br>. . . . . . . . . . . . . . . . . . . . . .<br>17|
||3.2|Interleaving Operation . . . . . . . . . . . . . . . . . . . . . . . . . .<br>18|
|||3.2.1<br>IBC Phases tradeoff . . . . . . . . . . . . . . . . . . . . . . .<br>18|
|||3.2.2<br>IBC Small-Signal Model . . . . . . . . . . . . . . . . . . . . .<br>19|
||3.3|Ideal Component Validation . . . . . . . . . . . . . . . . . . . . . . .<br>20|



V 

_TABLE OF CONTENTS_ 

|**4**<br>**Des**|**ign Ch**|**oices an**|**d Component Selection**|**22**|
|---|---|---|---|---|
|4.1|System|Archite|cture . . . . . . . . . . . . . . . . . . . . . . . . . . .|23|
|4.2|Power|Compon|ents . . . . . . . . . . . . . . . . . . . . . . . . . . . .|24|
||4.2.1|Inductor|Winding Design<br>. . . . . . . . . . . . . . . . . . . .|24|
|||4.2.1.1|Core Size Selection and Flux Density<br>. . . . . . . .|24|
|||4.2.1.2|Number of Turns . . . . . . . . . . . . . . . . . . . .|25|
|||4.2.1.3|Air-Gap Length<br>. . . . . . . . . . . . . . . . . . . .|25|
|||4.2.1.4|Winding Gauge and Litz Wire Configuration . . . .|26|
|||4.2.1.5|Power Loss and Thermal Estimation . . . . . . . . .|28|
|||4.2.1.6|Experimental Validation<br>. . . . . . . . . . . . . . .|29|
||4.2.2|Power S|emiconductors Devices . . . . . . . . . . . . . . . . .|30|
||4.2.3|RC snub|ber network . . . . . . . . . . . . . . . . . . . . . . .|31|
||4.2.4|Gate-dri|ver . . . . . . . . . . . . . . . . . . . . . . . . . . . .|32|
|||4.2.4.1|Typical Application . . . . . . . . . . . . . . . . . .|33|
|||4.2.4.2|Gate drive resistance with Turn-Off Diode<br>. . . . .|33|
||4.2.5|Capacit|or Stack Design<br>. . . . . . . . . . . . . . . . . . . . .|34|
|||4.2.5.1|Input Capacitor Stack . . . . . . . . . . . . . . . . .|34|
|||4.2.5.2|Output Capacitor Stack . . . . . . . . . . . . . . . .|34|
|4.3|Signal|Conditio|ning<br>. . . . . . . . . . . . . . . . . . . . . . . . . . .|36|
||4.3.1|Assessm|ent of Conditioning Circuitry Requirements<br>. . . . .|36|
||4.3.2|Input/O|utput Voltage Sensing<br>. . . . . . . . . . . . . . . . .|37|
||4.3.3|Hall-Eff|fect Sensor Conditioning Circuitry . . . . . . . . . . .|38|
|||4.3.3.1|Discrete differential amplifier using op-amps<br>. . . .|39|
|||4.3.3.2|Instrumentation amplifier . . . . . . . . . . . . . . .|40|
|4.4|Auxilia|ry Circu|itry . . . . . . . . . . . . . . . . . . . . . . . . . . . .|41|
||4.4.1|Low-vol|tage supplies . . . . . . . . . . . . . . . . . . . . . . .|41|
||4.4.2<br>4.4.3|ESD an<br>Heatsink|d Reverse Polarity Protection<br>. . . . . . . . . . . . .<br> evaluation<br>. . . . . . . . . . . . . . . . . . . . . . .|42<br>42|



VI 

_TABLE OF CONTENTS_ 

|||4.4.3.1<br>MOSFET Losses . . . . . . . . . . . . . . . . . . . .<br>42|
|---|---|---|
|||4.4.3.2<br>Diode Losses . . . . . . . . . . . . . . . . . . . . . .<br>43|
|||4.4.3.3<br>Heatsink Configuration . . . . . . . . . . . . . . . .<br>43|
||4.4.4|NTC temperature measurement . . . . . . . . . . . . . . . . .<br>44|
|4.5|Real c|omponents simulation . . . . . . . . . . . . . . . . . . . . . . .<br>45|
|4.6|Comp|arative Performance . . . . . . . . . . . . . . . . . . . . . . . .<br>46|
|**5**<br>**Con**|**trol T**|**echniques**<br>**49**|
|5.1|Averag|e Current Mode Control (ACMC) . . . . . . . . . . . . . . . .<br>49|
||5.1.1|Inner Current Loop<br>. . . . . . . . . . . . . . . . . . . . . . .<br>50|
||5.1.2|Outer Voltage Loop<br>. . . . . . . . . . . . . . . . . . . . . . .<br>51|
|5.2|Digita|l Control . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .<br>52|
||5.2.1|Digital PWM (DPWM) . . . . . . . . . . . . . . . . . . . . .<br>54|
||5.2.2|Digital Compensator Modeling . . . . . . . . . . . . . . . . .<br>56|
||5.2.3|Limit-Cycle Oscillations (LCO) . . . . . . . . . . . . . . . . .<br>56|
|5.3|Contr|oller Design . . . . . . . . . . . . . . . . . . . . . . . . . . . . .<br>57|
||5.3.1|Inner Current Loop<br>. . . . . . . . . . . . . . . . . . . . . . .<br>58|
|||5.3.1.1<br>K-factor Method . . . . . . . . . . . . . . . . . . . .<br>58|
||5.3.2|Outer Voltage Loop<br>. . . . . . . . . . . . . . . . . . . . . . .<br>59|
||5.3.3|Analog Compensator . . . . . . . . . . . . . . . . . . . . . . .<br>60|
||5.3.4|Digital Compensator . . . . . . . . . . . . . . . . . . . . . . .<br>60|
|5.4|Possib|le implementations<br>. . . . . . . . . . . . . . . . . . . . . . . .<br>61|
||5.4.1|Droop Method . . . . . . . . . . . . . . . . . . . . . . . . . .<br>62|
||5.4.2|Active sharing approaches . . . . . . . . . . . . . . . . . . . .<br>62|
|||5.4.2.1<br>Master-Slave . . . . . . . . . . . . . . . . . . . . . .<br>63|
|||5.4.2.2<br>Democratic . . . . . . . . . . . . . . . . . . . . . . .<br>63|
||5.4.3|Single-Sensor Current Sensing . . . . . . . . . . . . . . . . . .<br>64|



|**6**<br>**PCB Implementation**|**65**|
|---|---|



VII 

_TABLE OF CONTENTS_ 

||6.1|Floor Planning . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|65|
|---|---|---|---|
||6.2|STM32-F334R8 Microcontroller . . . . . . . . . . . . . . . . . . . . .|66|
||6.3|Power Blocks Schematics<br>. . . . . . . . . . . . . . . . . . . . . . . .|66|
|||6.3.1<br>Power Stage . . . . . . . . . . . . . . . . . . . . . . . . . . . .|66|
|||6.3.2<br>Leaf . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|67|
|||6.3.3<br>SuppliesLV<br>. . . . . . . . . . . . . . . . . . . . . . . . . . . .|68|
||6.4|Trace Width . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|69|
||6.5|Layout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|70|
||6.6|Cost Analysis . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|70|
|**7**|**Con**|**clusions and Future Works**|**72**|
|**A **|**Tra**|**nsfer Functions Manipulation**|**74**|
||A.1|Interleaved Boost Converter Small-Signal model . . . . . . . . . . . .|74|
||A.2|Computation of Boost _Gvd_(_s_) using SSA [13]<br>. . . . . . . . . . . . .|75|
||A.3|Computation of Boost _Gid_(_s_) . . . . . . . . . . . . . . . . . . . . . .|76|
||A.4|Computation of Boost _GioD_(_s_) [26] . . . . . . . . . . . . . . . . . . .|78|
|**Bi**|**bliog**|**raphy**|**79**|
|**De**|**dica**|**tions**|**82**|



VIII 

# **List of Figures** 

|1.1|Global hydrogen production trends and estimates by technology and<br>region from 2020 to 2025e. [1] . . . . . . . . . . . . . . . . . . . . . .|2|
|---|---|---|
|1.2|Global hydrogen production and demand breakdowns by source tech-<br>nology, sector, and region from 2020 to 2024. [1]<br>. . . . . . . . . . .|2|
|1.3|Investments in Hydrogen in EU countries. [3] . . . . . . . . . . . . .|3|
|1.4|Total Cost of Ownership (TCO) per kilometer for BET and FCET in<br>the US, EU, and China (2024 and 2030). [3] . . . . . . . . . . . . . .|4|
|2.1|Energy density comparison of available storage technologies [12] . . .|8|
|2.2|Proton exchange membrane fuel cells (PEMFC) single-cell structure.<br>[12] . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|9|
|2.3|(a) PEMFC Losses equivalent model. [5] (b) Third-order polynomial<br>fit of the Horizon H100 I-V curve.<br>. . . . . . . . . . . . . . . . . . .|10|
|2.4|PEMFC Equivalent SPICE® model. . . . . . . . . . . . . . . . . . .|11|
|3.1|N-phase Interleaved Boost Converter.<br>. . . . . . . . . . . . . . . . .|12|
|3.2|Boost Converter schematic [13]. . . . . . . . . . . . . . . . . . . . . .|13|
|3.3|Boost Converter current flow during 0_< t < TON_ [13]<br>. . . . . . . .|13|
|3.4|Boost Converter current flow during _TON < t < TS_ [13]<br>. . . . . . .|14|
|3.5|Volt-Second Balance Law in the Boost Converter case [13] . . . . . .|14|
|3.6|Thevenin equivalent circuit exploiting the transformer model for the<br>ideal Boost Converter [13]. . . . . . . . . . . . . . . . . . . . . . . . .|15|
|3.7|Boost Converter real behaviour [13] . . . . . . . . . . . . . . . . . . .|15|
|3.8|CCM Boost Converter Inductor Waveform [13]. . . . . . . . . . . . .|16|
|3.9|DCM Boost Converter Inductor Waveform [13] . . . . . . . . . . . .|16|



IX 

_LIST OF FIGURES_ 

|3.10|Boost Converter Output Capacitor and Diode current waveforms. [13]|17|
|---|---|---|
|3.11|Normalized Input Current Ripple for Interleaved Phases. . . . . . . .|17|
|3.12|(a) N-phase Interleaved Boost Converter. (b) _VGS_ and phase currents<br>in a two-phases IBC. . . . . . . . . . . . . . . . . . . . . . . . . . . .|18|
|3.13|Boost Converter DC (a) and AC (b) model. [13]<br>. . . . . . . . . . .|20|
|3.14|(a) Input Current (b) Output Voltage transient comparison, with<br>D=_D_nom.<br>. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|20|
|3.15|Ideal Conversion Ratio in CCM.<br>. . . . . . . . . . . . . . . . . . . .|21|
|3.16|(a) Input Current and (b) Output Voltage waveforms, with D=_D_nom,<br>using optimized inductance and capacitance values from Table 3.1 . .|21|
|4.1|System Block Diagram.<br>. . . . . . . . . . . . . . . . . . . . . . . . .|23|
|4.2|B65811 Core Dimensions (a) and winding packing inside window area<br>(b). [15] . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|26|
|4.3|(a) Air-gap effect on B-H loop. (b) Inductor equivalent magnetic<br>circuit. [15] . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|26|
|4.4|(a) Current density _J_ distribution against skin-depth. (b) Typical litz<br>wire section. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|27|
|4.5|B65811 core cross section. . . . . . . . . . . . . . . . . . . . . . . . .|29|
|4.6|Experimental setup and associated characteristic curves. . . . . . . .|30|
|4.7|Switching Losses assuming trapezoidal signal shape. [13] . . . . . . .|31|
|4.8|Snubber network applied to Conventional Boost Converter.<br>. . . . .|32|
|4.9|IR2110S Typical Application. . . . . . . . . . . . . . . . . . . . . . .|33|
|4.10|. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|33|
|4.11|(a) Input Capacitor Stack LTSpice model and related frequency re-<br>sponse (b).<br>. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|34|
|4.12|(a) Output Capacitor Stack LTSpice model and related frequency<br>response (b).<br>. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|35|
|4.13|Input (a) and Output (b) voltage sensing static characteristic. (0.1%<br>E192)<br>. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|38|
|4.14 <br>4.15|(a) ACS712 typical application. (b) Hall sensor static characteristic.<br> Differential Op-amp configuration analyzed. . . . . . . . . . . . . . .|38<br>39|



X 

_LIST OF FIGURES_ 

|4.16|(a) Typical instrumentation amplifier schematic. (b) Typical applica-<br>tion of INA326 instrumentation amplifier. . . . . . . . . . . . . . . .|40|
|---|---|---|
|4.17|INA326-based Hall Effect conditioning circuitry static characteristic,<br>considering 0.1% tolerance (E192 series). . . . . . . . . . . . . . . . .|41|
|4.18|Power delivery block diagram. . . . . . . . . . . . . . . . . . . . . . .|41|
|4.19|Diode average current (a) and Switch RMS current (b) for 0_._4_< D <_0_._8|44|
|4.20|Shared (a) and Single (b) heatsink per leaf. . . . . . . . . . . . . . .|44|
|4.21|Typical resistance as a function of temperature for an NTC tempera-<br>ture sensor<br>. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|45|
|4.22|Conventional Boost Converter with additional parasitic effects. . . .|46|
|4.23|(a) Input Current and (b) Output Voltage waveforms at steady state<br>with D=_D_nom.<br>. . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|46|
|4.24|Input Current Ripple (a) and Output Voltage Ripple (b) comparison.|47|
|4.25|Output Capacitor Stack RMS current (a) and Switch RMS current<br>(b) against Duty Cycle.<br>. . . . . . . . . . . . . . . . . . . . . . . . .|47|
|4.26|Efficiency _η_ (a) and Conversion Ratio _MCCM_ (b) comparison over a<br>restricted range of duty cycles.<br>. . . . . . . . . . . . . . . . . . . . .|48|
|5.1|Block diagram representation of ACMC for 2-phases IBC. [19] . . . .|50|
|5.2|Type-2 Analog Compensator. [13]<br>. . . . . . . . . . . . . . . . . . .|50|
|5.3|Typical organization of a digital current controller. [20]<br>. . . . . . .|53|
|5.4|(Top-left) effect of a too low sampling frequency on the reconstructed<br>signal; (Top-right) frequency domain interpretation of the aliasing<br>phenomenon; (Bottom) partial overlap of the spectrum spectrum<br>replicas due to aliasing.[20]<br>. . . . . . . . . . . . . . . . . . . . . . .|54|
|5.5|Block diagram of the digital current control loop with PI regulator. [20]|54|
|5.6|Analog (a) and Digital (b) PWM . . . . . . . . . . . . . . . . . . . .|55|
|5.7|Uniformly sampled PWM with single update mode: (a) general block<br>diagram of Zero-Order-Hold sampling model, (b) trailing-edge modu-<br>lation, (c) leading-edge modulation, (d) triangular carrier modulation.<br>[20] . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|55|
|5.8|(a) Euler integration method (forward and backward). (b) Trapezoidal<br>integration method (Tustin approximation). [20]<br>. . . . . . . . . . .|56|



XI 

_LIST OF FIGURES_ 

|5.9|Limit Cycle Oscillation due to invalid set-point among duty-cycle<br>possible outputs. [20] . . . . . . . . . . . . . . . . . . . . . . . . . . .|57|
|---|---|---|
|5.10|_Gid_(_s_) frequency response in MATLAB. . . . . . . . . . . . . . . . .|58|
|5.11|Bode plots in MATLAB of (a) Compensator and (b) Inner Current<br>Loop gain transfer functions.<br>. . . . . . . . . . . . . . . . . . . . . .|59|
|5.12|FIR filter followed by recursion. . . . . . . . . . . . . . . . . . . . . .|61|
|5.13|Block diagram of the closed-loop system with the virtual output<br>impedance path.<br>. . . . . . . . . . . . . . . . . . . . . . . . . . . . .|62|
|5.14|Fixed Master-slave current sharing control strategy.<br>. . . . . . . . .|63|
|5.15|Highest-Current Master-slave current sharing control strategy. . . . .|63|
|5.16|(a) Two-Phase Interleaved Boost Converter with Single Sensor based<br>Current Balancing Scheme. (b) Noise at the Switching Instances of<br>Inductor Current. [23] . . . . . . . . . . . . . . . . . . . . . . . . . .|64|
|6.1|Block disposition inside the board. . . . . . . . . . . . . . . . . . . .|65|
|6.2|System operated at (a) four sources in parallel, no interleaving op-<br>eration, (b) two sources in parallel using two interleaved phases. (c)<br>single source with four interleaved phases. . . . . . . . . . . . . . . .|66|
|6.3|Power Stage schematic.<br>. . . . . . . . . . . . . . . . . . . . . . . . .|67|
|6.4|Leaf schematic. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|68|
|6.5|Low-Voltage supplies schematic. . . . . . . . . . . . . . . . . . . . . .|69|
|6.6|Trace width evaluated through SaturnPCB toolkit ®. . . . . . . . . .|69|
|6.7|PCB layers. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|70|
|6.8|Board 3D Model. . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|71|
|6.9|Unit price per board versus production volume. . . . . . . . . . . . .|71|



XII 

# **List of Tables** 

|1.1|H2DRON Project Specifications . . . . . . . . . . . . . . . . . . . . .|6|
|---|---|---|
|3.1|Component design formulas for different phase configurations. . . . .|19|
|4.1|Inductor Stresses . . . . . . . . . . . . . . . . . . . . . . . . . . . . .|24|
|4.2|Inductor design parameters for different average currents. . . . . . .|28|
|4.3|Comparison of winding parameters between single wire and Litz wire.|28|
|4.4|Conductor losses and thermal performance evaluation. . . . . . . . .|29|
|5.1|Discretization Methods. [20] . . . . . . . . . . . . . . . . . . . . . . .|56|
|6.1|Phase Pinout and Channel Mapping . . . . . . . . . . . . . . . . . .|66|



XIII 

# **Acronyms** 

|FC|Fuel Cell.|
|---|---|
|HFCEV|Hydrogen Fuel Cell Electric Vehicle.|
|FCHEV|Fuel Cell Hybrid Electric Vehicle.|
|SMR|Steam Methane Reforming.|
|BEV|Battery Electric Vehicle.|
|RED|Renewable Energy Directive.|
|IMO|International Maritime Organization.|
|TCO|Total Cost of Ownership.|
|AFIR|Alternative Fuels Infrastructure Regulation.|
|IPCEI|Important Projects of Common European Interest.|
|UAV|Unmanned Aerial Vehicles.|
|IBC|Interleaved Boost Converter.|
|PEMFC|Proton Exchange Membrane Fuel Cell.|
|PEM|Proton Exchange Membrane.|
|SoH|State of Health.|
|DCM|Discontinuous Conduction Mode.|
|CRM|Critical Conduction Mode.|



XIV 

_Acronyms_ 

|ZVS|Zero-Voltage Switching.|
|---|---|
|ZCS|Zero-Current Switching.|
|SSA|State-Space Averaging.|
|ESD|Electro-Static Discharge.|
|TVS|Transient Voltage Suppression.|
|LUT|Look-Up Table.|
|NTC|Negative Temperature Coefficient.|
|RHPZ|Right-Half Plane Zero.|
|LHPZ|Left-Half Place Zero.|
|LHPP|Left-Half Plane Pole.|
|AP|Area Product.|
|RMS|Root Mean Square.|
|EMI|Electromagnetic Interference.|
|AWG|American Wire Gauge.|
|MLT|Mean-Length-Turn.|
|ADC|Analog-to-Digital Converter.|
|LDO|Low-dropout regulator.|
|ACMC|Average Current Mode Control.|
|PCMC|Peak Current Mode Control.|
|PWM|Pulse Width modulation.|
|DSP|Digital Signal Processor.|
|LSB|Least Significant Bit.|
|SNR|Signal-to-Noise Ratio.|



XV 

_Acronyms_ 

DPWM Digital Pulse Width Modulation. LCO Limit-Cycle Oscillations. HRTIM High Resolution Timer. 

XVI 

### **Chapter 1** 

# **Introduction** 

Over the past decades, the integration of hydrogen as a substitute of fossil fuels in applications ranging from power generation to industrial ecosystems, has been subject of discussion globally in terms of environmental impact due to the diverse methods of harvesting. The environmental benefit of hydrogen must be weighed against its production method; in fact, several processes are distinguished by their carbon intensity through a color-coded classification widely adopted by industry and public institutions. Currently, the most prominent methods are Steam Methane Reforming (SMR), electrolysis, and methane pyrolysis. The SMR method, traditionally resulting in ’Gray’ hydrogen, can be coupled with Carbon Capture, Utilization, and Storage (CCUS) systems to mitigate emissions, yielding what is known as ’Blue’ hydrogen. Furthermore, hydrogen produced via electrolysis is categorized based on its energy source: ’Green’ hydrogen, derived from renewables, offers the lowest carbon footprint but is subject to weather-dependent intermittency, whereas ’Pink’ hydrogen, utilizing nuclear energy, provides a more stable and continuous production profile. Finally, methane pyrolysis produces ’Turquoise’ hydrogen, where carbon is captured in a solid form, such as graphene, rather than as a gas. While low-emission hydrogen represents less than 1% of global supply, primarily from facilities with carbon storage capabilities, electrolysis-based production increased by 60% in 2024 to over 100 kt _H_ 2, led by capacity growth in China. Nonetheless, electrolysis still accounts for only one-sixth of low-emission output; this gap underscores the existing technical challenges in the sector. [1] 

1 

_Introduction_ 



<!-- Start of picture text -->
: a<br>Bio<br>zi [m=Middle East<br>om mmmChina<br>6 =2Europe<br>oa (=Canada<br>02 (aUnited States<br>*8 Foss Blectricity] Fossa El Fossil Elect Fossil Electric Fosst Eb Fossi Electric] ~~ Total<br>io [St |, | | |<br>Notes: CCUS = carbon capture, utilisation and storage: ROW= rest of world: 2025e = estimate for 2025, based on projects<br>‘Source: IE id n Pr yn Pr (September 2025).<br><!-- End of picture text -->

**Figure 1.1:** Global hydrogen production trends and estimates by technology and region from 2020 to 2025e. [1] 

In 2024, as we can see from the picture above, about 60% of the hydrogen market demand was fulfilled by ammonia production followed by the oil refining industry, reaching a demand of over 55 Mt, as can be observed in Fig. 1.2. New applications, such as mobility, transportation and bio-fuel production actually cover less than 1% of the global market demand. 



<!-- Start of picture text -->
By source technology By sector By region<br>z 60 60 60<br>Bw ai a<br>40 40 40<br>30 Ky 30<br>20 20 20<br>o 0 0<br>2020 2021 2022 2023 2024 2020 2021 2022 2023 2024 2020 2021 2022 2023 2024<br>Onsite - fossil wio CCUS Ammonia Methanol acme kas .<br>wOther ache Boner United States mestof World<br>Notes:carbon captureFossil wioandCCUSuse. =Infossilproduction fuels withoutby source carbon technology, capture, “Other”utilisation includes onsite and storage; productionFossil w CCU with =bioenergy, fossil fuelscarbon with<br><!-- End of picture text -->

**Figure 1.2:** Global hydrogen production and demand breakdowns by source technology, sector, and region from 2020 to 2024. [1] 

#### **1.1 Hydrogen in transportation** 

Over one-fourth of global CO2 emissions are connected to the transportation sector, alternative options are needed to minimize this impact through sustainable solutions such as the adoption of Hydrogen fuel cell electric vehicles (HFCEV), which with respect to conventional battery electric vehicles (BEVs), the fuel cells (FCs) are lighter and produce only water (as vapor) and heat as waste products. [2] 

2 

_Introduction_ 

Compared to traditional BEVs, HFC systems are lighter and the only waste products are water (as vapor), heat and thanks to its higher payload and faster refueling capabilities, HFCs are better suited for heavy-duty applications. While BEVs often require frequent and long charging sessions that increase dwell time (interval for which the vehicle stays stationary) costs, HFCEVs offer a refueling interval comparable to conventional internal combustion engines. The widespread implementation of hydrogen as an energy source in the transportation sector, has been encouraged by the large investments made by the institutions in the past decade, in fact from 2014 to 2024, IEA data assess that more than EUR 1.3 billion public funding has been injected in the heavy duty transportation ecosystem. In Europe, a substantial portion has been invested by France, accounting for roughly EUR 625 million. [3] [1] 



<!-- Start of picture text -->
@ NL; 3% a PT 2% mS, = 1% & SK; 096 JRC basedon IEA<br>@ LU; 0% m PL; m AT 4%<br>wT 0% 2h BBE 2% 2.1%<br>m@ areneHU ‘ = DK: 4%<br>w EE 0%<br>m EL; 0%<br>wES 2%<br>FI O%<br><!-- End of picture text -->

**Figure 1.3:** Investments in Hydrogen in EU countries. [3] 

Despite the fertile environment created by the governments through instruments such as EU Renewable Energy Directive (RED) and International Maritime Organization (IMO), the major limitation to its extensive market deployment of FC systems is related to its higher Total Cost of Ownership (TCO) with respect to battery electric vehicles (BEVs) considering hydrogen fuel prices at the pump, which vary considerably by region, including several other factors listed in Fig. 1.4. 

3 

_Introduction_ 



<!-- Start of picture text -->
30<br>& 27<br>gq 24<br>8fas<br>3B is<br>oooe i [|Et so PS sf OH= . °<br>te ee | I]<br>°°: fl a nan i A | on4 i<br>Uniedser  statescer] |] Europeaneet  Union}Fer] BetChina|FceT| setUnted States.cet] | Europeanser FceT|Union | setchinaFcET<br>2024 2030<br>Truck WFucl infrastructure Dwell cost Driver costs Other ODiesol TCO ~Sensiviy<br><!-- End of picture text -->

**Figure 1.4:** Total Cost of Ownership (TCO) per kilometer for BET and FCET in the US, EU, and China (2024 and 2030). [3] 

China maintains a central role in the global deployment of HFC vehicles, reaching 72% of market share in 2024, thanks also to support mechanisms such as the subsidies and incentives awarded to local governments for CNY 300 billion. In Europe, FC manufacturers are supported through instruments such as the AFIR (Alternative Fuels Infrastructure Regulation) and IPCEI (Important Projects of Common European Interest) programs which are willing to achieve the goal of 45% CO2 emissions reduction by 2030 in compliance with the EU regulation 2019/1242. [3] 

#### **1.2 Power Converters in Fuel Cell Applications** 

Most of the investments in the automotive industry are concentrated on Fuel Cell Hybrid Electric Vehicles (FCHEV), which are characterised by significant power ratings causing the DC-bus to achieve large voltages (700-800 V) to limit the size of the cables. For this reason, the literature has been mainly focused on the development of converters with high voltage conversion ratio through several techniques such as Voltage-Multiplier Cells, Magnetic Coupling and Multi-Stage structures. The only feasible choice for our application, in order to minimize the converter’s physical contribution, is to exploit configurations as the Interleaved structure to ease the design of the magnetic components and increase the lifetime of the FC stack by reducing its ripple current [4]. 

The main reasons voltage multiplier cells and magnetic coupling solutions are avoided are that they require a substantial number of components and heavy ferromagnetic cores, respectively, making them unsuitable for the application. 

##### **1.2.1 The Interleaved configuration** 

Fahrani et al. [5] have developed a three-phase Interleaved Boost Converter for 1.2 kW fuel cell powertrain . Hao et al. [6] extended the concept by increasing the number of input sources to three, paralleling a two-phase Interleaved configuration . 

4 

_Introduction_ 

The mismatch between our project specification and the previously cited papers lies in the different switching frequency and conversion ratio; in fact applying a 400 kHz switching frequency to a conventional boost topology, will cause significant disturbances and switching losses driven by the voltage and current peaks related to the parasitic elements causing Electromagnetic Interference (EMI), which can be discarded at lower switching frequencies. 

Therefore, soft-switching cells need to be employed to mitigate disturbances and switching losses. 

A smart solution to achieve the full zero voltage switching condition in turn-off mode and zero current switching in turn-on mode could be to drive the converter in Critical or Discontinuous Conduction Mode (CRM/DCM), allowing the main inductor to discharge completely before the next switching cycle without relying on additional complex structures, however, several challenges are encountered in terms of stresses on the components [7]. 

Among the soft-switching cells proposed in the literature, passive snubber cells are preferred over the active ones by the industry due to the hidden cost of the latters, which includes at least one auxiliary switching element that must be necessarily controlled by an additional gate driver, which makes the overall system cost rise [8] [9]. 

#### **1.3 Scope of the Thesis** 

This thesis is part of the research conducted within the LIGHT2DRONE project at the Universitat Politècnica de València, which focuses on the development of Unmanned Aerial Vehicles (UAVs) powered by alternative energy sources rather than fossil fuels in agricultural applications, exploiting hydrogen fuel cells (HFCs) as the main power source. Within this context, this thesis proposes the analysis, design and implementation of a four-phases Interleaved Boost Converter (IBC). 

The primary objective of this project is to develop a 400 kHz DC/DC power converter capable of stepping up the low voltage supplied by four distinct input sources to a higher level suitable for the system load, specifically a 12s LiPo-cell battery stack used to supply the drone motors. With this objective, the converter is designed to increase a nominal input voltage of 12 V, provided by a Horizon H100 8A Proton Exchange Membrane Fuel Cell (PEMFC), to a nominal output voltage of 44.4 V, nominally provided by the LiPo battery stack, while strictly limiting the input current ripple affecting the fuel cells to less than 10% with respect to the average current provided, to avoid harmful phenomena such as the catalyst loss, carbon-support corrosion in the electrodes due to Fuel Starvation phenomenon, which could compromise the integrity and lifetime performance of the hydrogen fuel cell [10]. 

By exploiting an Interleaved configuration, which distributes the current across 

5 

_Introduction_ 

multiple phase-shifted branches, the system significantly reduces the current ripple compared to a conventional Boost converter, while also enabling a reduction in the size of the passive components and improving the overall system reliability through the parallelization of the converter structure. The current state of the art of step-up converters based on the Interleaved structure, focuses mainly on power ratings above 1 kW applied to FCHEV applications. [11] 

Furthermore, isolated topologies are avoided in this case, despite their superior safety characteristics, to limit the already significant weight contribution of the transformers’ magnetic components. The challenge posed by a moderate power rating of 100 W is to find an appropriate trade-off among size, weight, and volume while achieving high efficiency, thereby enabling lighter thermal management systems and maximizing both the payload capacity (fertilizer, seeds, etc.) and flight time by accommodating a larger hydrogen storage tank. 

Finally, to ensure the stability of the given converter, the Average-Current Mode Control will be implemented using digital control techniques on an STM32 microcontroller by exploiting Hall-Effect sensors coupled with a suitable signal conditioning chain, establishing a solid base for future work. Through the integration of subsystems developed by other collaborators within the LIGH2TDRONE project, key operational parameters, such as the fuel cell’s State of Health (SoH) can be evaluated. 

##### **1.3.1 Project Specifications** 

Below are listed the primary design specifications and system requirements imposed by the LIGH2TDRONE project for the development of the converter. 

|Input Voltage Range|_Vin_|10_÷_18|V|
|---|---|---|---|
|Nominal Input Voltage|_Vin_|12(at 8A)|V|
|Output Voltage Range|_Vout_|34_._4_÷_50.4|V|
|Nominal Output Voltage|_Vout_|44.4|V|
|Maximum Input Power|_Pin,max_|100|W|
|Switching frequency|_fs_|400|kHz|
|Maximum Input Current|_Iin_|8|A|
|Maximum Input Current Ripple|∆_Iin_|10 %|A|
|Maximum Output Voltage Ripple|∆_Vout_|100|mV|



**Table 1.1:** H2DRON Project Specifications 

Alongside the electrical constraints summarized in Table 1.1, a fundamental systemlevel requirement imposed by the project is the capability to interface with multiple independent power sources operating in parallel. In fact, at the very core of a hybrid system lies the possibility to utilize and integrate different energy sources. This multi-source architecture is specifically required because such a topology is essential to provide fault protection and to significantly increase the overall reliability of the drone thanks to redundancy. 

6 

_Introduction_ 

#### **1.4 Thesis Outline** 

The document will cover the theoretical analysis of the interleaved structure and the sizing of the power components, followed by an overview of several control strategies. Simulations are performed to validate the theoretical calculations and provide a comparison between different numbers of phases. 

A short outline of Proton Exchange Membrane Fuel Cell (PEMFC) architecture, reaction mechanisms, and its equivalent circuit model is presented in Chapter 2. 

In Chapter 3, the interleaved boost converter topology is analyzed, beginning with the conventional design and extending the concept to a generic _N_ -phase converter, while also addressing its associated trade-offs and ideal steady-state operation. The design and simulation of the power components, alongside device selection, are showcased in Chapter 4, together with the necessary conditioning and auxiliary circuitry. 

Average Current Mode Control (ACMC) and concepts related to digital control design, including limitations and possible outcomes, are reported in Chapter 5. 

In Chapter 6, the choices behind the prototype implementation as well as a cost breakdown are presented. 

Finally, a brief discussion on the potential outcomes and future developments of the project is carried out, accompanied by a summary of the obtained results. 

To perform the mentioned analyses and designs, the following software programs and digital tools, included in the Politecnico di Torino student license program, are adopted: 

- **LTSpice:** A SPICE simulator from Analog Devices ® used to validate the theoretical assumptions and analyse the real behaviour of the components. 

- **WebPlotDigitizer:** A semi-automated data extraction software used to accurately digitize scanned plots and graphs, specifically employed to extract data points from the PEMFC static current-voltage (IV) characteristic curves. 

- **MATLAB:** from Mathworks ® used for numerical computations and control system modeling. 

- **Altium Designer:** The main EDA software employed to design the schematic and layout of the PCB prototype. 

- **SaturnPCB Toolkit:** A specialized utility tool providing rapid calculations for PCB design parameters, including trace width, conductor impedance and via current capacity. 

- **DigiKey:** The main component distributor adopted for the project. 

7 

### **Chapter 2** 

# **Fuel Cell Systems Overview** 

The actual systems built around conventional Li-Ion/LiPo batteries present a huge limitation in terms of flight time due to their low energy density of about 130-200 Wh/kg, which is almost half compared to the most advanced fuel cell commercial solutions that can exhibit from 250 to 540 Wh/kg making the Li-Ion/LiPo stacks obsolete in long-range applications such as heavy-duty transportation and general logistics. 



<!-- Start of picture text -->
Power density (W kg’)<br><!-- End of picture text -->

**Figure 2.1:** Energy density comparison of available storage technologies [12] 

Nevertheless, Lithium based batteries show several disadvantages with respect to FCs in terms of temperature adaptability, service life, environmental impact and finally, easy detectability by infrared sensors due to the large temperature increase during operation, thus limiting the application in the military industry. 

8 

_Fuel Cell Systems Overview_ 

#### **2.1 PEMFC Architecture and Reactions** 

The Proton Exchange Membrane Fuel Cell (PEMFC), like all kinds of FCs is an electrochemical device which converts the energy produced by the reaction between the fuel (hydrogen) and an oxidant (oxygen or air) into electrical energy, respectively identified as fuel electrode (anode) fed with hydrogen and the cathode electrode subjected to the oxidant flow. We can identify the electrochemical half-cell reactions in the following way: 



And then we can sum up the overall reaction as follows: 





<!-- Start of picture text -->
e<br>Anode Cathode<br>Ha i ; Air<br>Ht<br>Anode channel Cathode channel<br>encexcess HH: —> excess Air<br>Ee as +0<br>Ly Na<br>5 =<br>Proton exchange membrane Gas diffusion layer Catalyst layer<br><!-- End of picture text -->

**Figure 2.2:** Proton exchange membrane fuel cells (PEMFC) single-cell structure. [12] 

The structure employed for the cell is composed by the following elements: 

- Anode/Cathode Flow Channel: here the hydrogen is introduced. 

- Gas Diffusion Layer (GDL): the hydrogen gas diffuses after being pumped from the anode flow channel to react with the catalyst. 

- Electrodes (anode and cathode): thanks to the action of the catalyst (Pt, often employed), the hydrogen loses electrons faster, which circulate through the external circuit and transfers the proton to the Proton Exchange Membrane (PEM). 

9 

_Fuel Cell Systems Overview_ 

- Proton Exchange Membrane(PEM): it’s the core of the cell and it’s usually made of Nefion, this material lets only the flow of protons. 

- Cathode Flow Channel: here the oxygen is introduced, diffuses through the GDL and reacts with the protons coming from the Anode side to form water and heat, which are expelled through the same channel. 

#### **2.2 PEMFC Equivalent Circuit Model** 

During typical operating conditions, PEMFCs are subjected to activation ( _Vact_ ), ohmic ( _Vohmic_ ) and concentration losses ( _Vcon_ ). Under different loads, these losses cause the overall system to drift from a linear behaviour to a non-linear one, so an interpolating curve to describe precisely this behaviour must be defined. 



With the objective of making the simulation as close as possible to the real-world environment, it is necessary to extract the I-V curve trend (Fig. 2.3b) in order to be able to use it as a behavioural source in simulation programs like LTSpice®. 



<!-- Start of picture text -->
lec<br>3 | se<br>;<br>is<br>Erc 1 - H<br>(b)<br>(a)<br><!-- End of picture text -->

**Figure 2.3:** (a) PEMFC Losses equivalent model. [5] (b) Third-order polynomial fit of the Horizon H100 I-V curve. 

Finally, since the PEMFC source will be employed in a switched mode power supply (SMPS), the circuit model must take into account the transient response of the cell to reflect the real time constants related to the electrochemical reactions happening within it. 

Since the datasheet provided by the manufacturer does not include any kind of dynamic characteristics, it’s not possible to extract the response characteristics from the static curves (Fig. 2.3b), the choice is to proceed by fixing a series resistance, to simulate the ohmic losses, a parallel capacitance to comply with the switching transients and fix convergence issues in the simulator and a series inductance to 

10 

_Fuel Cell Systems Overview_ 

include the self-inductance of the cables, assuming a typical inductance per unit length of 100<sup>_<u>nH</u>_</sup> _m_<sup>(Fig. 2.4).</sup> 



<!-- Start of picture text -->
PEMFC static model Fuel Cell Cables Self Inductance<br>_ Ls<br>in<br>R1 500n<br>100m<br>Ci<br>Bi<br>in<br>V=a*(-I(B1))**3+b*(-1(B1))**2+c*(-I(B1))+dd<br>:param a=-0.026 b=0.3404 c=-1.87 dd=18.03<br><!-- End of picture text -->

**Figure 2.4:** PEMFC Equivalent SPICE® model. 

Typically, PEMFC sources exhibit slow dynamic responses, resulting in large time constants and high equivalent capacitance values. A 1 nF parallel capacitor is chosen, although higher values could be used, to speed up the simulations, acknowledging that in real operating conditions the steady-state regime may be reached in a longer period. 

11 

### **Chapter 3** 

# **Interleaved Boost Converter Analysis** 

Before analyzing the theoretical framework of the interleaving operation, it is necessary to examine the conventional boost converter. This preliminary step clarifies the converter’s working principle and highlights the main reasons for paralleling its structure. 



<!-- Start of picture text -->
lun lpn<br>LN DN<br>tle A Tbk }<br>Dk<br>lin > Ipo1 | lout<br>TN Dt Ic<br>in<br><!-- End of picture text -->

**Figure 3.1:** N-phase Interleaved Boost Converter. 

#### **3.1 Conventional Boost Converter** 

The Boost converter (Fig. 3.2) consists primarily of four elements: two semiconductor switches (a transistor SW and a diode D) and two passive components (an inductor L and an output capacitor _CO_ ). Because the transistor is in a low-side configuration, its driving circuit is simpler compared to alternative arrangements, such as high-side driving. 

12 

_Interleaved Boost Converter Analysis_ 



<!-- Start of picture text -->
ig @ | i © i, GUD i,<br>: men ad Ta Db<br>he v, ise J a Rick fs a<br>Uy G -| VV You (an R<br>sw<br><!-- End of picture text -->

**Figure 3.2:** Boost Converter schematic [13]. 

For the sake of simplicity, during the converter analysis the following assumptions are considered: 

- Capacitor _Cin_ is neglected 

- Active elements as SW and D are ideal 

- Passive elements parasitics are neglected 

- The converter works in ciclostationary condition 

##### **3.1.1 Steady-State Analysis** 

At time 0 _< t < TON_ , the switch is turned on thus the voltage across the inductor is _V_ in, while the voltage across the diode is _−V o_ , so it’s reverse biased. The inductor current rises with constant slope of _V_ in _/L_ until the switch opens. 



<!-- Start of picture text -->
veVu D<br>iyree:@ fj s SOi, @i OFF i,ReamesOUD i,<br>. —<br>te x vv, if \<br>Yu G V4=0 Ve R}\v,<br><!-- End of picture text -->

**Figure 3.3:** Boost Converter current flow during 0 _< t < TON_ [13] 

Since the inductor current cannot change instantaneously, during _TON < t < TS_ (total switching interval) the current must find a path to continue to flow by turning on the diode. 

13 

_Interleaved Boost Converter Analysis_ 



<!-- Start of picture text -->
° ix, @i ~veVrV,© i © ON> i, CHI,<br>t oe.SS a a on<br>ta avy v0 7 Ves Le<br>: th<br>‘ _<br><!-- End of picture text -->

**Figure 3.4:** Boost Converter current flow during _TON < t < TS_ [13] 

In cyclostationary conditions, the average voltage across the inductor must be null (Volt-Second balance principle), so it’s possible to find the relationship between the duty cycle and the conversion ratio when the inductor is driven in Continuous Conduction Mode (CCM) as: 



<!-- Start of picture text -->
v(t) |<br>4 + ur<br>silt)<br>Slope: — | ee<br>Firedara ai<br>t/T,<br>0 D 1<br><!-- End of picture text -->

**Figure 3.5:** Volt-Second Balance Law in the Boost Converter case [13] 



So it’s possible to extract the ideal voltage gain _MCCM_ as: 



A brief analytical analysis reveals that the conversion ratio unrealistically explodes when the duty cycle is equal to 1. This is because the expression currently lacks consideration for parasitic losses. A short circuit at the output must also be considered because it will result in the input being shorted as well. 

To account for parasitic effects, the conversion ratio can be derived by extracting the Thévenin equivalent circuit from the converter’s transformer model. This model consolidates the ohmic losses of the wire winding into an equivalent series resistance, 

14 

_Interleaved Boost Converter Analysis_ 

_Rind_ . 



<!-- Start of picture text -->
A Mak >) |<br>ee ,<br>4. Se | + ><br>; Rely, => v.G) Rely,<br>= —<br><!-- End of picture text -->

**Figure 3.6:** Thevenin equivalent circuit exploiting the transformer model for the ideal Boost Converter [13]. 



From the formula (3.3), it’s possible to acknowledge that the internal losses of the inductor critically limit the achievable output voltage. 



<!-- Start of picture text -->
M<br>‘a F<br>9 a<br>8<br>iIdeal behavior /<br>i Actual behavior<br>:<br>a a a a<br><!-- End of picture text -->

**Figure 3.7:** Boost Converter real behaviour [13] 

Incorporating parasitic losses in the realistic model introduces an error term that causes the conversion ratio to drop to zero as _D_ approaches 1, reflecting a physically plausible response. Mechanically, when _D_ = 1, the low-side switch remains continuously closed (ON). Consequently, the inductor stores energy without transferring it to the load, preventing the output capacitor from charging and causing the output voltage ( _VO_ ) to fully discharge to zero (Fig. 3.7). 

##### **3.1.2 Inductor Analysis** 

Finally it’s possible to define a condition for the converter in order to operate in Continuous Conduction Mode (CCM) or Discontinuous Conduction Mode (DCM) depending on the requirements. By analysing the current shape it’s possible to define the key parameters of the converter, in our case, since the inductor is connected in 

15 

_Interleaved Boost Converter Analysis_ 

series with the input, the inductor current ripple definition is directly related to the input current ripple. 

###### **3.1.2.1 CCM Operation** 



**Figure 3.8:** CCM Boost Converter Inductor Waveform [13]. 

Additionally, in order to satisfy the CCM condition the inductor current must be non-null; therefore we need to impose the following boundary condition. 



The required inductance L, must be greater than the critical inductance expressed in (3.5). Where R is the load resistance (defined also as _Vo/Io ≈ Vo_<sup>2</sup><sup>_/Po_),Dtheduty</sup> cycle and _fs_ the switching frequency. 

For the sake of completeness, the DCM operating mode discussion is also carried on to acknowledge its advantages and disadvantages over the CCM condition. 



<!-- Start of picture text -->
V(t) 4 swon sort yor<br>aye Fe we<br>Boe cee (DOr<br>9<br>; VV<br>i(t)<br>Slope: |+¥e ee<br>pe T L<br>Iyaax :<br>0 PS t<br>l se ><br>LL 1 4%<br>< ><br>T,<br><!-- End of picture text -->

**Figure 3.9:** DCM Boost Converter Inductor Waveform [13] 

16 

_Interleaved Boost Converter Analysis_ 

As illustrated in Fig. 3.9, operating in Discontinuous Conduction Mode (DCM) allows the inductor to fully discharge during each switching cycle. Consequently, to deliver the same average current at a given switching frequency, the converter exhibits higher current ripples concentrated within a narrower time interval compared to Continuous Conduction Mode (CCM) resulting in a higher inductor current RMS component. 

For this reason, the converter is operated in CCM to limit stress on the input source, thereby avoiding harmful phenomena such as fuel starvation. Conversely, DCM allows the inductor current to discharge naturally to zero, thus enabling the implementation of soft-switching techniques [7] [14]. 

##### **3.1.3 Output Capacitor** _CO_ 

In a boost converter, the diode current is discontinuous and causes high stress on the output capacitor during the OFF interval, which must sustain huge AC components. 

The behavior of the output voltage ripple in a boost converter is determined by the type of output capacitor selected. For an electrolytic capacitor, we can assume a 



<!-- Start of picture text -->
siti(t<br>fe) D 1 t/T,<br>‘o(t)<br><!-- End of picture text -->

**Figure 3.10:** Boost Converter Output Capacitor and Diode current waveforms. [13] 



<!-- Start of picture text -->
aos |<br>Be<br>° \ V.<br><!-- End of picture text -->

**Figure 3.11:** Normalized Input Current Ripple for Interleaved Phases. 

resistive behaviour at the switching frequency. In this case, the output voltage ripple is directly dependent on the capacitor’s Equivalent Series Resistance (ESR), the relationship as follows. 



Conversely, if a ceramic capacitor is chosen, the frequency range over which it behaves as a resistor is narrower. To find the required capacitance value, one must integrate the charge over the OFF-interval, utilizing the flat-top approximation. 



17 

_Interleaved Boost Converter Analysis_ 

#### **3.2 Interleaving Operation** 

To overcome the bottlenecks specified before in this chapter, the Interleaved Boost Converter (IBC) architecture is introduced by connecting _N_ identical converter legs in parallel sharing a common input source and output capacitor. The current rating of each switching device is scaled down to _Iin/N_ , significantly reducing conduction losses and thermal stress. Instead of driving all switches synchronously, a phase-shift angle of _θ_ =<sup><u>360</u></sup> _N_<sup>_◦_</sup> is applied between consecutive gate signals to achieve the interleaved operation. In (3.7), it is possible to find the expression for the input current ripple 



<!-- Start of picture text -->
toy tow<br>Hac LNA! DN low<br>- nT | Beltor |_ \igts<br>™N Dt le<br>Inv<br><!-- End of picture text -->



<!-- Start of picture text -->
(a)<br><!-- End of picture text -->



<!-- Start of picture text -->
45<br>aw<br>40<br>‘0<br>Vosi (V) 0<br>aw)baitswn<br>0<br>0<br>0 2 4 6 8 10<br>Time (us)<br><!-- End of picture text -->

**(b)** 

**Figure 3.12:** (a) N-phase Interleaved Boost Converter. (b) _VGS_ and phase currents in a two-phases IBC. 

given a number of phases _N_ , where _x_ = 1 _,_ 2 _, . . . , N_ , assuming the same inductor value _Lx_ = _L_ and duty cycle _dx_ ( _t_ ) = _d_ ( _t_ ) in steady-state operations [6]. 



Increasing the number of phases offers several key advantages. The individual phase currents sum up, and the applied phase shift causes the total current ripple to reduce significantly. Furthermore, having additional phases increases the equivalent output ripple frequency, which allows for the use of smaller filtering capacitors. 

From a dynamic perspective, the reduced effective inductance pushes the Right-Half Plane Zero (RHPZ) to higher frequencies compared to a conventional single-phase boost converter, thereby expanding the achievable loop bandwidth. 

##### **3.2.1 IBC Phases tradeoff** 

The performance advantages offered by the interleaved architecture must be weighted according to the specific application. Increasing the number of phases _N_ leads to higher circuit complexity and a greater component count, including gate drivers and sensing circuitry. Consequently, the benefits resulting from ripple reduction and the downsizing of individual passive components must be carefully evaluated and 

18 

_Interleaved Boost Converter Analysis_ 

weighed in terms of costs, occupied volume and efficiency. [6] 

Table 3.1 shows the derived design expressions of the inductance and capacitance values with the related commercially available component according to the E12 tolerance series. Notably, (3.1) shows how interleaving allows for smaller individual component sizes as the number of phases increases. The actual component stresses in terms of RMS currents, input current ripple, and output voltage ripple, will be quantitatively evaluated and validated through the simulation results presented in Chapter 4. 

|**Configuration**|**Design Formula**|
|---|---|
|**Conventional Boost**|_L_= <sup>_Vin,min·Dmax_</sup><br>∆_IL·fs_<br>_→L_= 27_µ_H (_±_10% E12)<br>_C_ =<br>_Io·D_<br>∆_v·fs _<sup>_→C_ = 47</sup><sup>_µ_F (</sup><sup>_±_10% E12)</sup>|
|**2-Phase IBC**|_L_= max<br>(<br>_Vout_<br>∆_iin·fs _<sup>_·_ (2</sup><sup>_d −_1)</sup><sup>_·_ (1</sup><sup>_−d_)</sup><br>)︂<br>_→L_= 22_µ_H (_±_10% E12)<br>_C_ =<br>_Io·D_<br>∆_v·_2_fs _<sup>_→C_ = 22</sup><sup>_µ_F (</sup><sup>_±_10% E12)</sup>|
|**3-Phase IBC**|_L_= max<br>(<br>_Vout_<br>∆_iin·fs _<sup>_·_ (3</sup><sup>_d −_2)</sup><sup>_·_ (1</sup><sup>_−d_)</sup><br>)︂<br>_→L_= 12_µ_H (_±_10% E12)<br>_C_ =<br>_Io·D_<br>∆_v·_3_fs _<sup>_→C_ = 18</sup><sup>_µ_F (</sup><sup>_±_10% E12)</sup>|
|**4-Phase IBC**|_L_= max<br>(<br>_Vout_<br>∆_iin·fs _<sup>_·_ (4</sup><sup>_d −_3)</sup><sup>_·_ (1</sup><sup>_−d_)</sup><br>)︂<br>_→L_= 3_._9_µ_H (_±_10% E12)<br>_C_ =<br>_Io·D_<br>∆_v·_4_fs _<sup>_→C_ = 12</sup><sup>_µ_F (</sup><sup>_±_10% E12)</sup>|



**Table 3.1:** Component design formulas for different phase configurations. 

To determine the optimal number of interleaved stages, a comparative analysis must be performed focusing on efficiency, input current ripple, and output voltage ripple. Typically, in the industry the number of phases does not exceed three due to the hidden costs related to the gate drivers and sensing circuitry specified earlier. 

##### **3.2.2 IBC Small-Signal Model** 

Before proceeding to the modeling phase, several assumptions must be made. Since this interleaved structure consists of parallel-connected boost converter stages sharing a common input source and output node, it can be demonstrated that the small-signal model of an IBC is equivalent to that of a conventional boost converter. 

The demonstration obtained using the State-Space Averaging (SSA) method is presented in A.1 and shown under the assumptions that all phase inductances are equal, so _L_ 1 = _L_ 2 = _..._ = _LN_ = _Lph_ and that each converter leg is phase-shifted by exactly _θ_ =<sup><u>360</u></sup> _N_<sup>_◦_.</sup> 

After several steps, it can be demonstrated that the small-signal model of the IBC 

19 

_Interleaved Boost Converter Analysis_ 

coincides with the conventional Boost Converter as follows: 



where _L_ eq =<sup>_L_</sup> _N_<sup><u>ph</u>,leadingtothefollowingequivalentcircuit:</sup> 



<!-- Start of picture text -->
L® Lb © |.<br>- te<br>ol Je<br>V V<br>°<br><!-- End of picture text -->



<!-- Start of picture text -->
1,@ Lo loi,<br>eh MA<br>ve oS ‘ ae<br>® l@<br><!-- End of picture text -->



<!-- Start of picture text -->
(a) (b)<br><!-- End of picture text -->

**Figure 3.13:** Boost Converter DC (a) and AC (b) model. [13] 

#### **3.3 Ideal Component Validation** 



<!-- Start of picture text -->
2 sha fhae AA ia<br>Reedy arias uty :<br>Sead VW ae hb ade yt<br>EE ii We H<br>Sg q I¥ u¥ u¥ q<br>3<br>8.0 <>) 2-Phases IBC<br>=<br>4PhasesIBC<br>499 49294 4996.—HR 5000<br>Time (Us)<br><!-- End of picture text -->



<!-- Start of picture text -->
455<br>IRCRSCURNURREStiddd ry<br>S40 NP SEIS Shh aa) SY<br>8, it wt tt it<br>Bust tt it ra rt<br>kc= oud4 pbiRy baitkp boii<br>zo<br>43S === 2-PhasesIBC<br>= = 4PhasesIBC<br>a ee ee<br>Time (us)<br><!-- End of picture text -->



<!-- Start of picture text -->
(a) (b)<br><!-- End of picture text -->

**Figure 3.14:** (a) Input Current (b) Output Voltage transient comparison, with D= _D_ nom. 

As evident from Fig. 3.14, increasing the number of phases from 2 to 4 results in an increase in the ripple frequency and a consequent reduction in its amplitude. Furthermore, the conversion ratio changes its behavior, as can be observed in Fig. 3.15. 

20 

_Interleaved Boost Converter Analysis_ 



<!-- Start of picture text -->
—— Conventional Boost ‘<br>15.0 i<br>--- 2-Phases IBC i<br>12.5 4]7'7 4-Phases IBC i<br>= 10.0<br>o<br>= 75<br>5.0<br>2.5<br>0.0<br>0.0 0.2 0.4 0.6 0.8 1.0<br>Duty Cycle (D)<br><!-- End of picture text -->

**Figure 3.15:** Ideal Conversion Ratio in CCM. 

An interesting analysis, conducted using the values shown in Table 3.1, can be done by using the optimized values for the inductor and the output capacitor. In fact, the interleaved structure can be also exploited to reduce the single components dimension to keep the input current ripple under a specific treshold, exploiting the ripple reduction capabilities of the topology. 



<!-- Start of picture text -->
2010 IMAITRAD ARDARRARARARA<br>= 100<br>Eos ===) 2-Phases IBC<br>z =:—_4-PhasesIBC<br>Bs A AA AA A<br>so \ .<br>4990 499249944995 «49985000<br>‘Time (us)<br><!-- End of picture text -->



<!-- Start of picture text -->
S 9) SINIYNININAAASSRARNNNNANMANMASA®<br>Po<br>i a== 2-Phases<br>S a =T_ 4 PhasesIBCIBC<br>6 MORALE AP LAS<br>ats | i + i i<br>4990 4992-4994 «499649985000<br>Time (Hs)<br><!-- End of picture text -->



<!-- Start of picture text -->
(a) (b)<br><!-- End of picture text -->

**Figure 3.16:** (a) Input Current and (b) Output Voltage waveforms, with D= _D_ nom, using optimized inductance and capacitance values from Table 3.1 . 

A variation of the overall conversion ratio with respect to the fixed components case is shown in Fig. 3.16b as the value of inductance and output capacitance is reduced. In this case, the inductor could no longer be a bulky component, as its individual contribution is combined with the other phases to achieve current ripple reduction, thereby allowing a reduction in its physical size. 

21 

### **Chapter 4** 

# **Design Choices and Component Selection** 

According to the project specifications, the system has four distinct inputs and a single common output. The worst-case condition in terms of components’ stress corresponds to the case where four sources operate in parallel; therefore, the standard boost converter design procedure is followed to comply with the primary requirement of ∆ _iin <_ ∆ _iin,_ max. The design of the power components along with the stresses and dynamic characteristics is discussed in Section 4.2 . After the design of the power components, the analysis proceeds with the signal conditioning together with the tolerance analysis both for input voltage, output voltage and phase current in section Section 4.3 . 

To ensure safe and stable operation, housekeeping features such as temperature sensing, thermal management and ESD protection are discussed in Section 4.4 . 

Finally, a simulation comparison between different numbers of phases using models provided by the manufacturers is presented via SPICE simulations in Section 4.5. 

22 

_Design Choices and Component Selection_ 

#### **4.1 System Architecture** 



<!-- Start of picture text -->
Heatsink<br>Temperature<br>TerminalsOutput ___) output Voltage CC y<br>ADC STM32F334<br>TUE6LSB<br>ToyeInput Voltage CC ‘conv 8.52 us,<br>Buck 5 V<br>Hall Effect Conditioning<br>Circuitry<br>EE odepwes <1 aaxav<br>Sensor<br>occ<br>| ~~ _ |<br><!-- End of picture text -->

**Figure 4.1:** System Block Diagram. 

This section outlines the overall system architecture of the proposed interleaved topology. The system is structured into two coupled domains: the high-power conversion stage and the low-voltage control domain, which includes the signal conditioning circuitry. Figure 4.1 illustrates the top-level block diagram, showing the interaction between the basic power block, digital regulation and the gate drivers. 

For the voltage sensing, resistive divider networks followed by a buffer stage have been employed, while Hall effect sensors are exploited for the current sensing action. Both the input and output voltages are sensed, even though only the output voltage is currently used for the control loop; the input voltage measurement can be leveraged in future development phases to detect the presence of an input source and consequently include feedforward compensation into the control loop. 

To properly supply all the circuitry inside the board, a cascade of low voltage converters is mandatory to step down the voltage coming from the output stack with the intention of eliminating the need of external sources. At the core of the scheme, the STM32 microcontroller provides the necessary PWM signals to control the gate drivers and utilizes its internal ADC to sample the voltages and currents required for the control algorithm. In this project, the control loop is implemented via a digital PI controller to ensure stable regulation. 

23 

_Design Choices and Component Selection_ 

#### **4.2 Power Components** 

##### **4.2.1 Inductor Winding Design** 

Since it’s quite challenging to find components with both precise inductance values and high stresses capability typical of high power environments; the choice is to proceed with a custom design inductor adopting the Area-Product (AP) design procedure, which relates the maximum magnetic energy to the physical requirements of the magnetic core. The component stresses used for the procedure are described in Table 4.1 below: 

|**Parameter**|**Value**|
|---|---|
|Minimum Inductance (_L_min)|24_._2 _µ_H(Table 3.1)|
|Maximum Current Ripple (∆_iL,_max)|0_._8 A (10%_I_in,max)|
|Maximum Current (_IL_max)|8_._8 A|
|Minimum Current (_IL_min)|7_._2 A|
|Average Current (_IL_avg)|8 A|



**Table 4.1:** Inductor Stresses 

To choose the inductance value, the CCM and the current ripple condition must be satisfied. The relative expression can be extracted from the formula 3.4: 



As specified in (3.2.1), the first commercially available value for the inductor, according to the E12 series is 33 _µH_ in order to ensure proper design reproducibility. 

Finally, the RMS current value is extracted assuming a purely triangular waveform with and average value of _IL,avg_ and amplitude ∆ _iL,max_ , given the following relationship: 



###### **4.2.1.1 Core Size Selection and Flux Density** 

To properly select the minimum core dimension, the Area-Product (AP) defined as the product between the cross-sectional area and the window area, must be evaluated according to different situations. The first area product _AP_ 1 constraint regards the limit related only to the copper loss. 



Where _K_ represents the core filling factor, which is the fraction of the window area occupied by the conductor, typically this value lies around 0 _._ 7 but drops to 0 _._ 55 when considering the non ideal packing of the windings. _Bsat_ (set to 0 _._ 45 _T_ ) is the 

24 

_Design Choices and Component Selection_ 

magnetic saturation flux density. The second term _AP_ 2 concerns the combined loss between the copper and the magnetic core. 



Where _Kh_ and _Ke_ (4 _·_ 10<sup>_−_5</sup> and 4 _·_ 10<sup>_−_10</sup> ) take into account the hysteresis and the eddy currents effects respectively. The maximum between _AP_ 1 and _AP_ 2 will set the minimum area product requirement of the selected magnetic core. 

###### **4.2.1.2 Number of Turns** 

The number of turns _N_ must be chosen properly to avoid magnetic saturation, limit the losses inside the core and obtain the correct inductance value. 



The first condition ensures that the core can withstand the maximum current impulse without going into saturation, whereas the second equation determines the number of turns required to manage the continuous ripple current, expressed through ∆ _Bmax_ , which represents the maximum magnetic flux variation that can be supported during steady-state operation. The value of the effective area _Ae_ is specified in the datasheet of the selected core material. Relationship (4.5) ensures robust operation in both cases where core losses are either negligible or significant: 



It’s important to acknowledge that when the core losses are significantly bigger than the copper losses since the switching frequency plays a huge role in the definition of the maximum magnetic flux variation ∆ _Bmax_ through the hysteresis and eddy current coefficients. 

###### **4.2.1.3 Air-Gap Length** 

Since the inductor is subjected to high DC currents, a physical discontinuity in the ferromagnetic core must be included to increase the reluctance thus, enlarge the linear region for which the inductor behaves as expected without core saturation. 



After selecting the number of turns _N_ , the air-gap can be evaluated. Since this precise dimension cannot typically be adjusted directly, it is implemented in practice using multiple layers of insulating paper. The main problems related to the introduction of such discontinuity range from Electromagnetic Interference (EMI) to the higher 

25 

_Design Choices and Component Selection_ 



<!-- Start of picture text -->
SY 4o><br>2 <0<6<br>2 ZN<br>t Si FLBm OX<br>aVfl ehi ‘ 32<br>Ne ee<br>Ke 4<br>~9.8 min._<br>23.2-0.9<br><!-- End of picture text -->



<!-- Start of picture text -->
Wire, bare area |__ BRERRRRRRRRRooo<br>DPA sssscsssssss|<br>Core window<br>areaW<br><!-- End of picture text -->



<!-- Start of picture text -->
(b)<br><!-- End of picture text -->



<!-- Start of picture text -->
(a)<br><!-- End of picture text -->

**Figure 4.2:** B65811 Core Dimensions (a) and winding packing inside window area (b). [15] 

copper losses due to the increased number of turns, but this operation is mandatory to store sufficient magnetic energy to ensure proper operation over a switching cycle. 



<!-- Start of picture text -->
8 ge"<br>Ferriteair gap without V1 Ferrite/,4,7“en with<br>ag airgap<br>y 4<br>“7<br>“ye<br>“er<br>it<br>eer<br>Compan of tees lot sfrtecae with and<br>‘ithoutan oe<br><!-- End of picture text -->



<!-- Start of picture text -->
.<br>had » 2<br>% mnie g<br><!-- End of picture text -->



<!-- Start of picture text -->
(b)<br><!-- End of picture text -->

**(a)** 

**Figure 4.3:** (a) Air-gap effect on B-H loop. (b) Inductor equivalent magnetic circuit. [15] 

###### **4.2.1.4 Winding Gauge and Litz Wire Configuration** 

To choose the winding section, thermal requriments must be taken into account. Indeed, the gauge of the cable can be determined by selecting a safe current density which keeps the temperature increase below a limit of ∆ _Tmax <_ 30<sup>_◦_</sup> . The thermal power lost by the core _Pcore_ is what dictates the choice of the maximum current density _Jmax_ . _Pcore_ is assumed negligible with respect to _Pcu_ , due to constructive reasons, in this case the wire cross section can be reduced because a larger temperature rise in the core is admitted, leading to an overall reduction of the window occupied area. 



26 

_Design Choices and Component Selection_ 

The (4.7) relates empirically the current density with the area product _AP_ under the assumption of a 30<sup>_◦_</sup> _C_ temperature rise. From _Jmax_ the minimum cross section can be extracted using the _IL,rms_ as described in (4.8): 



Assuming a circular cross section, the diameter and the related commercial AWG (American Wire Gauge) can be extracted. The gauge selected for this application is 15, corresponding to a wire diameter of 0 _._ 145 cm. 



Since the inductor is operating at high frequency, skin effect must be taken into account. This effect introduces AC losses inside the conductor together with the DC losses already present due to the non-null conductor resistivity _ρ_ : 



Where _ρ_ 0 is the copper resistivity at 25<sup>_◦_</sup> _C_ while _α_ is the linear thermal coefficient specific for copper. After evaluating the resistivity at the maximum operating temperature of _T_ = 60<sup>_◦_</sup> _C_ ( _Ta_ = 30<sup>_◦_</sup> _C_ , ∆ _Tmax_ = 30<sup>_◦_</sup> _C_ ) exploiting (4.10), the skin depth at the operating frequency can be calculated as follows: 



Consequently, the equivalent surface where the current is actually flowing shrinks, increasing the resistance as first Ohm’s law states _R_ = _ρ S_<sup>_<u>l</u>_.Typically,if</sup><sup>_dcable>_2</sup><sup>_δ_,</sup> Litz wire should be employed due to the strict confination of the current at the boundaries, as highlighted in Fig. 4.4a. 



<!-- Start of picture text -->
Strand insulation<br>0 t/e 1<br>Strand<br>TI J Sh Bundle<br>(a)<br>(b)<br><!-- End of picture text -->

**Figure 4.4:** (a) Current density _J_ distribution against skin-depth. (b) Typical litz wire section. 

The Litz wire approach, although theoretically suitable to mitigate skin effect, appears 

27 

_Design Choices and Component Selection_ 

||**Convent**|**ional BC**|**2-**|**IBC**|**4-I**|**BC**|
|---|---|---|---|---|---|---|
|**Parameter**|E12|Minimum|E12|Minimum|E12|Minimum|
|Switching Frequency|||4|00 kHz|||
|Inductor Ripple Current|0.66 A|0.8 A|1 A|1.26 A|4 A(CCM)|4 A(CCM)|
|Inductance L|33 µH|27.3 µH|22 µH|17.4 µH|3.9 µH|3.8 µH|
|Turns|13|11|6|6|4|4|
|Core Fit|**X**|**V**|**V**|**V**|**V**|**V**|
|Airgap|0.15 mm|0.14 mm|0.07 mm|0.05 mm|0.13 mm|0.14 mm|
|Max AWG|14 (1.|62 mm)|17 (1|.15 mm)|19 (0.9|1 mm)|



**Table 4.2:** Inductor design parameters for different average currents. 

impractical due to the excessive number of strands required to meet current density constraints. The AWG choice for the Litz strands is in fact strongly constrained by the skin depth _δ_ , with the maximum acceptable conductor diameter typically limited to about 2 _δ_ , which further restricts the available options. Laboratory limitations reinforce this constraint, suggesting that a conventional single-wire solution may be more viable, even at the cost of higher AC losses. 

||**Single wire**|**Litz wire**|
|---|---|---|
|AWG|14|32|
|Number of turns|9|9|
|Number of strands|1|64|
|Length needed|19.58 cm<br>12.8 m|(25 cm _×_ 64 strands)|



**Table 4.3:** Comparison of winding parameters between single wire and Litz wire. 

###### **4.2.1.5 Power Loss and Thermal Estimation** 

The Mean-Length-Turn (MLT) approximation is adopted to evaluate the overall conductor length and calculate the relative resistive losses. 



Where _r_ core _,_ min and _r_ core _,_ max values can be extracted from the core cross section shown in Fig. 4.5. 

Power losses are evaluated differently depending on whether Litz wire is employed. If we consider the full copper cable solution, the effective area due to the skin effect shrinks in a ring shape of thickness _δ_ . On the other hand, in a Litz wire section, each strand radius must be strictly smaller than _δ_ to avoid skin effect, this operation leads to having a full surface exploited for the conduction, with the cost of having a complex structure of the strands (4.13). 

Finally, the power loss of the conductor and the core due to the magnetic flux 

28 

_Design Choices and Component Selection_ 



**Figure 4.5:** B65811 core cross section. 

variation are described below: 



In the end, the powers sum up to evaluate the relative temperature rise exploiting the thermal resistance definition through the Area-Product (AP) factor. 



In particular, is interesting to observe, how the IBC structure reduces the thermal stress on the inductor as the number of phases increases, as highlighted in Table (4.4). 

|**Conductor**|_L_|= 27_µ_**H**||_L_|= 22_µ_**H**||_L_|= 3_._9_µ_**H**||
|---|---|---|---|---|---|---|---|---|---|
||**Copper Loss **|**Temp. Rise **|**Status **|**Copper Loss **|**Temp. Rise **|**Status **|**Copper Loss **|**Temp. Rise **|**Status**|
|Litz|0_._343 W|13_._45 <sup>_◦_</sup>C|V|0_._068 W|12_._9 <sup>_◦_</sup>C|V|0_._041 W|11_._8 <sup>_◦_</sup>C|V|
|No Litz|1_._009 W|41_._06 <sup>_◦_</sup>C|X|0_._163 W|16_._6 <sup>_◦_</sup>C|V|0_._075 W|13_._2 <sup>_◦_</sup>C|V|



**Table 4.4:** Conductor losses and thermal performance evaluation. 

###### **4.2.1.6 Experimental Validation** 

To effectively determine the behavior of a custom power inductor, there are two main approaches. The quickest is to use the manufacturer’s model, which has a certain limit of validity, while the most precise is to perform a hardware characterization using lab instruments to measure impedance over a wide frequency range. Since operating frequencies for upcoming projects will reach 1MHz, it is critical to extract a model that is reliable at these higher scales. 

To properly characterize the dynamic behavior of the 1.52 mm insulated wire inductor, the Frequency Response Analyzer PSM3750 produced by Newtons4th Ltd. is used. 

29 

_Design Choices and Component Selection_ 



**(a)** Experimental setup with PSM3750. 



**(b)** Series Inductance plot. 



**(c)** Linear approximation of the B-H hysteresis plot. 

**Figure 4.6:** Experimental setup and associated characteristic curves. 

In particular, the inductance value at the switching frequency (400 kHz) is evaluated, we must acknowledge that in this condition the inductor is polarized with a current much lower with respect to the current expected by the design (8 A). By looking closely at the series inductance graph in Fig. 4.6b, the measured inductance value (91 _._ 56 _µH_ ) is higher than expected because the inductor is operating at a low current bias, keeping it in the high-permeability region of the B-H curve. As highlighted by the plot below, the self resonant frequency is fixed around 2 _._ 5 MHz. 

##### **4.2.2 Power Semiconductors Devices** 

The recommended specifications for the switch, in order to comply with the high switching frequency and thermal requirements, are specified below: 

- _ID ≥_ 50 A: continuous drain current. 

- _VDS ≥_ 150 V: drain-source voltage. 

- _RDS_ ( _on_ ) _≤_ 10 mΩ: static drain-source on-resistance. 

- _tr, tf ≤_ 10 ns: rise/fall times. 

30 

_Design Choices and Component Selection_ 

The BSC110N15NS5ATMA1 from Infineon-Technologies® is selected. Regarding the diode, based on the list below, the Schottky V60D100C from Vishay® is chosen. 

- 100 _V < VBD <_ 180 _V_ : breakdown voltage. 

- _IF,avg ≈_ 50 A: average forward current. 

- _CJ ≈ CO,sw_ : junction capacitance comparable to the output capacitance of the switch. 

##### **4.2.3 RC snubber network** 



<!-- Start of picture text -->
Vultl,<br>MOSFET( ON MOSFET OFF<br>+<br>v<br>P 7 0 To H To t<br>sy | IVR,une }-——y |<br>20<br>= ae ae Ital [te t<br>walt a |<br>0 tool [tm italfete ?<br>pit)s | | parabolaeeeIntervals le|<br>-Velo/eviAR:4=VYAR, HA Pox=Voulon \Cee (\<br>0 t<br><!-- End of picture text -->

**Figure 4.7:** Switching Losses assuming trapezoidal signal shape. [13] 

In Fig. 4.7 is shown the effect of the current and voltage waveforms finite slope on the instantaneous power dissipated by the switch. Switching losses occur due to the superposition of voltage and current together with the parasitic inductances of the terminals that cause a resonating behaviour which increase the overall dissipated power. Soft-switching topologies can be employed in order to both shift the voltage and current edges to reach the Zero-Voltage-Switching (ZVS) and Zero-CurrentSwitching (ZVS) during turn-on and turn-off respectively. Another solution to reach this conditions naturally could be to drive the power converter in DCM [7], but as explained in the previous chapters, this choice would dramatically increase the stress across all components. [16] [17] 

The evaluation of the suitable soft-switching topology is left as a future insight while for this project, to damp the ringing and reduce the switching losses, a snubber RC network is added on the output side of the switch. 

31 

_Design Choices and Component Selection_ 



<!-- Start of picture text -->
la wa Di<br>{Ld}<br>R3<br>{R_SN}<br>PULSE(0 10 0 2n 2n {D*Ts} {Ts})R2 |“ jy<br>BSC11qN1SNSSSC_L1<br>2 c12<br>Rser=10 en<br>Vi<br><!-- End of picture text -->

**Figure 4.8:** Snubber network applied to Conventional Boost Converter. 

As a rule of thumb, the snubber capacitance can be selected as 3 _· C_ oss where _C_ oss is the output capacitance of the selected switch. Finally, a resistance is needed to adjust the snubber resonating frequency. For the sake of simplicity, a parasitic inductance of 10 nH is assumed, then it’s possible to determine the minimum damping resistance by setting the quality factor from (4.18) to<sup><u>1</u></sup> 2<sup>,resulting in a 4</sup><sup>_._7Ωresistor according to</sup> the E12 commercial series. From simulations, is possible to evaluate the component stresses under different stack voltages and duty ratios, in particular the maximum RMS current flowing in the snubber network is _≈_ 600mA, due to the high current peaks. For which is possible to determine the power dissipated by the snubber resistance as: 



A 3W resistor is chosen to handle this power rating. 

##### **4.2.4 Gate-driver** 

Even though the converter is not bidirectional, the use of a full-bridge gate driver is justified by the possibility of achieving bidirectionality in the future by substituting the diode with a high-side power transistor. The rise and fall times must be selected accordingly to balance EMI and control response, since a higher slope of the current flowing through a metal trace causes proportional excitation of the inherent parasitic inductance and consequently induces large voltage spikes. 

The order of magnitude of the given switching frequency imposes to keep the rise and fall times below (10 _÷_ 20) ns. For this reason the IR2110S gate driver IC from Infineon® is adopted as it exhibits symmetrical 2A source/sink current. In (4.17) is possible to check the compatibility with the chosen switch: 



32 

_Design Choices and Component Selection_ 

where _Q_ g,sw is the total gate charge of the selected switch. 

###### **4.2.4.1 Typical Application** 



<!-- Start of picture text -->
° G) T<br>HIN HIN, Vs ° TO<br><!-- End of picture text -->

**Figure 4.9:** IR2110S Typical Application. 

From the datasheet, it’s possible to acknowledge that the gate driver logic is 3.3V compatible so it can be controlled directly by the STM32 pins without the need of any additional level shifters. In our case, since only the low-side section of the gate driver is used, the bootstrap diode highlighted in Fig. 4.9 is not needed. 

Finally, all the high-side input terminals are tied to ground to avoid any floating voltages compromising the gate driver operation. 

###### **4.2.4.2 Gate drive resistance with Turn-Off Diode** 



<!-- Start of picture text -->
Gate Driver tp Rod Rint «ty<br><!-- End of picture text -->

**(a)** Switching Parasitics circuit modeling. 



<!-- Start of picture text -->
ut Rod Ot |<br>D1<br><!-- End of picture text -->

**(b)** Simple Turn-Off Speed Enhancement Circuit 

###### **Figure 4.10** 

The gate resistance is added to avoid overshoot during the switching action while the diode is used to reduce the turn-off interval. The trace parasitic inductance is assumed equal to 10 nH (1 nH/cm) while the input capacitance and minimum internal gate resistance informations can be found in the manufacturer’s datasheet as 2080 pF and 0 _._ 9 Ωrespectively. 

33 

_Design Choices and Component Selection_ 

This model resembles a typical RLC resonator whose quality factor is: 



To prevent overshoot, _Q >_ <u>12</u><sup>overdampstheresonator,slowingtheoverallrise</sup> time. Alternatively, a balanced tradeoff is found by selecting _Q_ =<sup><u>1</u></sup> 2<sup>toachievethe</sup> critical damping condition, resulting in a gate drive resistance of 3 _._ 9 Ω. Ideally, the anti-parallel diode acts as a short when the driver’s output goes to 0. To further modulate the width of the turn-off interval, a series resistance in the diode branch can be introduced. [18] 

##### **4.2.5 Capacitor Stack Design** 

An array of capacitors must be employed to distribute the stress across all elements and increase overall system reliability. In particular, the stack must filter out higherorder harmonics in order to provide a smooth waveform at the output. The objective is to keep the response between 100 kHz and 1 MHz as flat as possible while complying with the capacitor thermal capabilities in terms of RMS current. To provide a reliable representation of the stack’s behaviour, the RLC model of the capacitor is employed. 

###### **4.2.5.1 Input Capacitor Stack** 

The source side is the least stressed in terms of current since we are dealing with a current-fed converter. Given that the maximum voltage provided by the fuel cell is 18 V, 50 V-rated capacitors are employed. 



<!-- Start of picture text -->
0 Sa<br>g noe 2 ae<br>i AE OY 4<br>y lca cis co cr p Arty og<br>(a)<br>(b)<br><!-- End of picture text -->

**Figure 4.11:** (a) Input Capacitor Stack LTSpice model and related frequency response (b). 

###### **4.2.5.2 Output Capacitor Stack** 

The output capacitor is subjected to a huge current stress, so the choice is to use multiple electrolytic capacitors to handle the high RMS current, while additional ceramic capacitors are used to flatten the response over the specified frequency range, 

34 

_Design Choices and Component Selection_ 

as done in the previous section. Considering that the maximum voltage of the LiPo battery stack at the output can reach 50.4 V, 100 V-rated capacitors are chosen. 

By taking into account the output voltage ripple and equivalent series resistance (ESR) constraints highlighted in Table 1.1, the required capacitor parameters can be derived as follows: 



Where _Iout_ is defined ideally as: 



Where _D_ min and _D_ max expression is: 



While ∆ _iO_ = _I_ max _≈ I_ out by looking at the output capacitor waveform in Fig. 3.10, adopting the flat-top approximation. So, the output capacitor value and equivalent series resistance can be obtained as: 





<!-- Start of picture text -->
> ae re re ls las<br>at<br><!-- End of picture text -->



<!-- Start of picture text -->
(a)<br><!-- End of picture text -->



<!-- Start of picture text -->
SPR 90.<br>g Fad<br>ffl t / 3<br>J ‘ i CC 20 2<br><!-- End of picture text -->



<!-- Start of picture text -->
(b)<br><!-- End of picture text -->

**Figure 4.12:** (a) Output Capacitor Stack LTSpice model and related frequency response (b). 

35 

_Design Choices and Component Selection_ 

#### **4.3 Signal Conditioning** 

##### **4.3.1 Assessment of Conditioning Circuitry Requirements** 

Without any signal conditioning, resolution is degraded, making it difficult to distinguish between closely spaced samples. The STM32 ADC has a maximum resolution of 12 bits with a 3.3 V reference, yielding a single LSB value of 0.81 mV and resulting in a total number of levels equal to: 



Before going deep in complex calculations we need to evaluate the minimum current change related to one step of duty cycle, that is defined by the following expression: 



Where ARR is the value of the auto-reload register of the High Resolution Timer (HRTIM) used to generate the 400kHz PWM square waves, defined as: 



Then, to evaluate the minimum variation of the sensed signal, in this case the phase current, the relative expression must be developed as follows: 



With (4.26) the minimum inductor current variation due to a discrete variation of the duty cycle can be defined: 



The value extracted in (4.27) can be translated in terms of variation of the hall effect sensor voltage as: 



After including all kinds of uncertainty terms related to the conditioning circuitry, the hall device and the total unadjusted error (TUE) of the STM32 ADC it will be possible to check whether the condition (5.9) is satisfied to avoid the limit cycle 

36 

_Design Choices and Component Selection_ 

oscillations phenomena described in (5.2.3) . 

_δV_ sense, L = _δ_ TUE + _δ_ cond + _δ_ hall _→ δV_ sense, L = _δ_ TUE + ( _δ_ INA,gain + _δ_ INA,offset) + _δ_ hall _→ δV_ sense, L = 6 LSB + (0 _._ 2 % _V_ INA,max + 100 _µV_ ) + 1 _._ 5 % _V_ hall,max (4.29) _→ δV_ sense, L = 4 _._ 8 mV + (0 _._ 2 % 3 _V_ + 100 _µV_ ) + 1 _._ 5 % 3 _V → δV_ sense, L _≈_ 6 mV 

Since the conditioning chain uncertainty exceeds the ADC resolution ( _δV_ sense, L _> δV_ ADC), the ADC performance in terms of Effective Number of Bits (ENOB) is degraded. This causes the ultra-fine duty cycle resolution to be completely masked by the uncertainty, making it ineffective for this application. 

Several solutions can be employed. The most effective would be to average the samples via oversampling; however, there is little room to increase the sampling frequency since the system is already sampling at 400 kHz, given that the maximum sampling frequency provided by the STM32 ADC is approximately 1 MS/s. A second solution could involve performing a precise laboratory characterization of the conditioning circuitry and ADC to reduce systematic uncertainties; nevertheless, this approach is impractical and time-consuming in an industrial context. 

For this reason, to induce a measurable current shift that exceeds the uncertainty floor the HRTIM clock frequency must be modified to obtain ∆ _V_ hall _> δI_ L,sense. 

##### **4.3.2 Input/Output Voltage Sensing** 

A voltage divider followed by a buffer is employed to sense the voltage at both the input and output. The buffer is needed in order to decouple the resistor divider from the analog front-end of the ADC inside the microcontroller. The gain of the divider is set to reach a maximum output of 3 V when the maximum voltage is applied, 50.4 V and 18 V respectively for the output and input sides, to avoid including any additional protection circuitry that would raise the cost of the conditioning section. A first-order low-pass filter, with a cutoff frequency of 10 kHz, is included to limit noise injection from nearby traces. In the picture below, the static characteristic of the conditioning circuitry is shown, considering a tolerance of 0.1% (E192 series) which results in a maximum deviation of ∆ _vsense_ = 6mV at its output. 

The selected Op-Amp for both the input and the output sensing is the general purpose Rail-to-Rail TLV9151IDBVR from Texas Instruments ®. The SPICE simulations highlighted above are done using a standard MCP6022 Op-Amp since the model of the selected device was not available. 

37 

_Design Choices and Component Selection_ 



<!-- Start of picture text -->
a a<br>i<br>“<br>10 u 2 B rr 1s 16 v<br>Veemec [V]<br><!-- End of picture text -->



<!-- Start of picture text -->
Py<br>2<br>0 2 “ 6 ry 0<br>Vuro [V]<br><!-- End of picture text -->



<!-- Start of picture text -->
(a) (b)<br><!-- End of picture text -->

**Figure 4.13:** Input (a) and Output (b) voltage sensing static characteristic. (0.1% E192) 

##### **4.3.3 Hall-Effect Sensor Conditioning Circuitry** 

The ACS712 Hall-effect linear current sensor from Allegro MicroSystems® has been adopted for this project, although it is designed to handle higher current ratings than those expected in the converter. As the datasheet suggests, it exhibits an offset of<sup>_<u>V</u>_</sup><sup><u>CC</u></sup> 2 with a current sensitivity of 66 mV/A (ACS712ELCTR-30A-T). A filter capacitor is included, as recommended by the manufacturer, to set the bandwidth to 50 kHz. 



<!-- Start of picture text -->
oe) +5V<br>7 Vor<br>b lip, | VOC 8<br>p+ VIOUT re) Care<br>ne AcS712 Ta ote<br>3) p_ FiTeR|®<br>= (<br>Gy |5—g 1 0F<br><!-- End of picture text -->



<!-- Start of picture text -->
3,054) [-----------22--2-2-eeeese<br>V_cond<br>BOW [i-ss-2c-seeceeseeseeeeed<br>noon '<br>25V '<br>84A |<br><!-- End of picture text -->



<!-- Start of picture text -->
(a) (b)<br><!-- End of picture text -->

**Figure 4.14:** (a) ACS712 typical application. (b) Hall sensor static characteristic. 

If the sensor is placed in series with the inductor, as typically done in the research, the worst-case condition is dictated by the maximum current at the output of the fuel cell plus half the maximum expected ripple ( _≈_ 8 _._ 4 A), for which the output voltage of the conditioning circuitry is set to 3 V in order to provide a safety margin for the microcontroller ADC. Alternatively, if it’s placed in series with the diode to measure the output current of the leaf, the maximum current decreases compared to the previous case, and the gain required to achieve the same characteristic is larger, as can be deduced from Fig. 4.14b. The current resolution must be improved through the use of proper conditioning circuitry to enhance the control action and avoid oscillatory and unstable behaviours. 

38 

_Design Choices and Component Selection_ 

To achieve the characteristic described in Fig. 4.14b, the idea is to subtract the existing offset with a specific reference and amplify the sensor signal to extend its static characteristic. The chosen reference voltage cannot be set to exactly 2.5 V due to the uncertainty in the sensor offset voltage, which ranges within _±_ 30 mV as derived from the total output error specified in the manufacturer’s datasheet. Therefore, 2.4 V was selected; although this choice may introduce a residual offset at the output of the conditioning circuitry, this can be compensated easily in firmware. Two solutions are analysed in the following sections: 

- **Discrete differential amplifier using op-amps:** A simple solution, but avoided due to the difficulty of precisely balancing the potentiometer ratios. 

- **Instrumentation amplifier:** The chosen approach due to its well-defined gain equation and lower component count, despite its higher cost. 

###### **4.3.3.1 Discrete differential amplifier using op-amps** 



<!-- Start of picture text -->
a<br>8<br>z<br>3<br>RG<br>{we(Rval1, tol, 0) Ri {wk(Rhallf, tol,5)}<br>>,L RS 3<br>Vv Mcp60p1 [uz<br>{we(Rval2, {we(Rhall1, tol,4)} ><br>tol,1) °? vi R4 ‘McP6021<br>Vv {we(Rhallg,{we(Rhall2,tol, tol,329% ° v<br>v<br><!-- End of picture text -->

**Figure 4.15:** Differential Op-amp configuration analyzed. 

From basic circuit theory, it’s well known that to achieve proper differential operation, the ratios _R_ 6 _/R_ 5 and _R_ 4 _/R_ 3 must be equal to perform the subtraction without introducing additional error. The problem with this solution, as specified earlier, is related to the extreme precision required to mitigate these additional terms. 

where: 



So if _V_ ref =<sup>_<u>VCC</u>_</sup> 2 is set (Fig.4.15), the output of the conditioning circuitry would be ideally: 



39 

_Design Choices and Component Selection_ 

In practice, non-idealities such as the offset and gain errors of the current sensor must be taken into account. The use of fixed resistor values is not recommended since the user would not be able to correctly adjust the characteristic to make it suitable for the application. In this case, at least two potentiometers must be employed to set both the resistor ratio and the differential gain. Due to the complex adjustment of the potentiometers, the proposed configuration was abandoned. 

###### **4.3.3.2 Instrumentation amplifier** 



<!-- Start of picture text -->
Yo Ry Ry<br>Ry<br>R > Vp<br>R,<br>re ‘gain out<br>ve R2 R—_<br><!-- End of picture text -->



<!-- Start of picture text -->
ave tur Single-supplyRRThis > 00Kmay produce foroperation full ouputhighermay swinginput requirereferred<br>offset votage, See Offset Votage<br>vo ae Dit, and Creal Values or deta<br>car) Ry<br>R, é OVo Fitered<br>cot<br>A thr G=2R/R,)<br>4 Vp 1000<br>Re a” -<br>= "common?!<br><!-- End of picture text -->



<!-- Start of picture text -->
(a) (b)<br><!-- End of picture text -->

**Figure 4.16:** (a) Typical instrumentation amplifier schematic. (b) Typical application of INA326 instrumentation amplifier. 

Despite the higher cost, the choice for the project is to use an instrumentation amplifier to ease the setting of the gain without the need to balance resistor ratios. The INA326EA manufactured by Texas Instruments® is the device employed for this application. The differential gain is set by a potentiometer ( _R_ 1) and a fixed resistor ( _R_ 2). In Fig. 4.16b, two additional capacitors are used to fix the bandwidth at 1 kHz, but this value is not suitable for the control loop. 

To achieve the characteristic highlighted in the figure 4.14b, it must be compared with the gain expression of the instrumentation amplifier as follows: 



According to the manufacturer’s datasheet, different values of gain can be selected. Since the gain ranges between 5 and 10, a 1 kΩcermet potentiometer is used as _R_ 1 as it exhibits a theoretical infinite resolution thanks to its continuous film, and _R_ 2 is fixed at 3 _._ 3 kΩ. To set the pole position to 50 kHz, _C_ 2 is chosen as 1 nF. Regarding the output RC filter, _RO_ is set to 100 Ωand _CO_ is set to 100 pF to mitigate high-frequency spikes. 

40 

_Design Choices and Component Selection_ 



<!-- Start of picture text -->
Tolerance Analysis<br>— Rul<br>3.0 —— Run?<br>— Ru3<br>2.5 Run4<br>Run5<br>>. 20<br>a<br>g 1.5<br>><br>1.0<br>0.5 M7<br>0.0<br>2.5 2.6 2.7 2.8 29 3.0 3.1 3.2<br>Viraut [V]<br><!-- End of picture text -->

**Figure 4.17:** INA326-based Hall Effect conditioning circuitry static characteristic, considering 0.1% tolerance (E192 series). 

#### **4.4 Auxiliary Circuitry** 

This section is dedicated to all the housekeeping parts of the system in order to provide stable and safe operation. 

##### **4.4.1 Low-voltage supplies** 



<!-- Start of picture text -->
LiPo stack<br>“<br>“s<br>conditioningstaCircuitry+ stis2 L00 33v<br><!-- End of picture text -->

**Figure 4.18:** Power delivery block diagram. 

The main problem related to the structure in Fig. 4.18 is its reliability; in fact, if a single converter fails, the entire chain operation will be corrupted. In this case, I 

41 

_Design Choices and Component Selection_ 

decided to preserve the efficiency of the chain by reducing the gap between the input and output voltages of each block. 

The LMR36510 1 A step-down converter from Texas Instruments® is adopted. The internal LDO of the STM32 is used to provide the 3 _._ 3 V supply to all the conditioning circuitry. In this case, the Nucleo board must remain connected even during the conditioning circuitry adjustment phase. The choice of using a buck converter for the first stage is driven by the large voltage difference between the LiPo stack and the board’s internal circuitry operating at 3.3 V. Therefore, a suitable trade-off was achieved by introducing an intermediate conversion stage to reduce thermal and electrical stress on the internal STM32 LDO. A linear regulator to step down the voltage from 12 V to 5 V could have been used; however, efficiency was prioritized despite the higher cost. 

##### **4.4.2 ESD and Reverse Polarity Protection** 

The internal logic is exposed to electrostatic discharge (ESD) due to the presence of external metallic contacts. Suitable protection components are needed to clamp and absorb high-voltage spikes to ensure system reliability. TVS (Transient Voltage Suppression) diodes, specifically the Littelfuse Inc.® P6SMB20CA ( _V_ BD = 21 V) and the STMicroelectronics® SM6T56CA ( _V_ BD = 53 _._ 2 V), placed at the input ( _V_ IN,max = 18 V) and output ( _V_ OUT,max = 50 _._ 4 V) sides, respectively. Finally, the Diotec Semiconductor® SK2040YD2R Schottky diode is used in series with the source to prevent reverse polarity at the input. 

##### **4.4.3 Heatsink evaluation** 

To precisely determine the heatsink requirements, the main power contributions, such as switching and conduction losses, must be evaluated. The RMS and average current values are extracted from conventional boost converter simulations in LTSpice, shown in Fig. 4.19, using the SPICE models provided by the manufacturers. 

###### **4.4.3.1 MOSFET Losses** 

The conduction losses of the switch can be described as follows: 

_P_ cond,SW = _R_ DS,on(hot) _·I_ SW,rms<sup>2</sup><sup>_≈_2</sup><sup>_·R_DS,on</sup><sup>_·I_</sup> SW,rms<sup>2</sup><sup>_≈_2</sup><sup>_·_11</sup><sup>_m_Ω</sup><sup>_·_(10</sup><sup>_A_)2</sup><sup>_≈_2</sup><sup>_W_(4.32)</sup> 

A separate discussion must be provided for switching losses, which can be calculated using a trapezoidal signal shape approximation, as shown in Fig. 4.7. In (4.33), the turn-on and turn-off power losses are evaluated by averaging the instantaneous power over a switching cycle. 

42 

_Design Choices and Component Selection_ 



Where _t_ ON and _t_ OFF are the turn-on and turn-off intervals respectively, both assumed 10ns. 



Leading to an overall power loss of: 



The datasheet of the BSC110N15NS5 states that _RthJA_ = 50 K/W leading to a junction temperature change of: 



Which is exactly the maximum junction temperature of the given device, so to keep it under safe operating conditions is necessary to introduce a heatsink to reduce the overall thermal resistance and limit the maximum junction temperature. 

###### **4.4.3.2 Diode Losses** 

The diode can be modeled as a constant voltage source _VF_ during _T_ ON, so the power dissipated is: 



As done in the previous section, it’s possible to evaluate the maximum junction temperature from the junction-to-ambient thermal resistance _RthJA_ = 45K/W in the V60D100C-M3/I datasheet provided by the manufacturer. 



From (4.37), we note that the diode junction temperature is well below a reasonable threshold with respect to its maximum junction temperature ( _T_ J,max = 150<sup>_◦_</sup> C), thus avoiding the need for an external heatsink during normal operation. 

###### **4.4.3.3 Heatsink Configuration** 

To properly choose the thermal resistance value, the maximum temperature of the system is fixed at 100<sup>_◦_</sup> C in order to gain a safe margin with respect to the maximum junction temperature and achieve negligible performance degradation. Since the heatsink is shared between the switch and the diode for mechanical reasons, the 

43 

_Design Choices and Component Selection_ 



<!-- Start of picture text -->
SO _ Be<br>NJ<br>|A<br><!-- End of picture text -->



<!-- Start of picture text -->
4<br>Lo<br><!-- End of picture text -->

**(a) (b)** 

**Figure 4.19:** Diode average current (a) and Switch RMS current (b) for 0 _._ 4 _< D <_ 0 _._ 8 



<!-- Start of picture text -->
é<br>ae ™<br>/ ‘= r TT<br>ale is = . A<br><!-- End of picture text -->



<!-- Start of picture text -->
—<br>Few “e 8<br>5 es = T ir<br><!-- End of picture text -->



<!-- Start of picture text -->
(a) (b)<br><!-- End of picture text -->

**Figure 4.20:** Shared (a) and Single (b) heatsink per leaf. 

maximum heatsink temperature for the most critical component must be evaluated. 

_T_ env,max = _Tj,max − Ploss,SW · Rθ_ JAMOS = 150<sup>_◦_</sup> _C −_ 2 _._ 4 W _·_ 50<sup>_◦_</sup> _C/W_ = 30<sup>_◦_</sup> _C_ (4.38) 

The project specifications require the configuration shown in Fig. 4.20a. However, I couldn’t find any commercial heatsinks with such dimensions. A suitable heatsink is only available for the configuration in Fig. 4.20b. As shown in (4.39), choosing an ambient temperature of 30<sup>_◦_</sup> C would result in an ideal thermal resistance of 0<sup>_◦_</sup> C/W. Because a component with this value is commercially unavailable, I assume an ambient temperature of 25<sup>_◦_</sup> C to allow for the selection of a suitable commercial model. 



To comply with the constraint above, a 29 mm × 29 mm × 30 mm heatsink from Fischer Elektronik® (Digikey: 5496-ICKS29x29x30-ND) has been selected. 

##### **4.4.4 NTC temperature measurement** 

A NTC (Negative Temperature Coefficient) thermistor is included to check the temperature of one of the heatsinks during steady-state operation. 

44 

_Design Choices and Component Selection_ 

The heatsink temperature can be measured through a surface temperature sensor, which exhibits slow gradients. NTCs, as the name suggests, exhibit negative rate of change of electrical resistance versus temperature, as shown in Fig. 4.21. In particular, the coefficient B is a material constant and controls the slope of the _R_ NTC characteristic, which can be described by the formula (4.40). Also B changes with temperature, so in manufacturer’s datasheets it’s often defined between 25<sup>_◦_</sup> _C_ and 85<sup>_◦_</sup> _C_ as _B_ 25/85. 



Or in general, since we are interested in the temperature measurement, the inverse relationship is evaluated as follows: 





<!-- Start of picture text -->
log R (0) wee<br>Rysf-----—<br>!<br>\ = 3740 K<br>1<br>\<br>\<br>\<br>: B=4570K’<br>\<br>1<br>\<br>oO<br>-25 0 25 50 75 100 125<br>TEC)<br><!-- End of picture text -->

**Figure 4.21:** Typical resistance as a function of temperature for an NTC temperature sensor 

To avoid overloading the microcontroller with this high order polynomial function, a Look-Up Table (LUT) can be built relating the voltage measurement to the actual temperature value. The values of the coefficients A, B, C, D, A1, B1, C1 and D1 can be found in some datasheets, a second order approximation is the solution adopted by the industry typically. 

Due to the slow response of the Vishay® NTCALUG01T and the fact that temperature is not a crucial information, the sampling frequency can be relaxed to avoid overloading the microcontroller. A voltage divider with a 10 _k_ Ωresistor is used to sense the characteristic of the NTC. 

#### **4.5 Real components simulation** 

Parasitic effects such as the self inductance of the cables and terminations can be included in the simulation in order to achieve a more comprehensive view on the 

45 

_Design Choices and Component Selection_ 

transient behaviour of the given converter. 



<!-- Start of picture text -->
mt veal<br><!-- End of picture text -->

**Figure 4.22:** Conventional Boost Converter with additional parasitic effects. 

However, the cost is given by the time needed to perform the additional computations. In particular, in Fig. 4.23b it’s shown the effect of the inductive spikes during the switching actions which can degrade the overall performance. The latters, can induce EMI and reduce efficiency of the system, for this reason, a snubber network must be employed in order to damp the oscillations. 



<!-- Start of picture text -->
08 ERE<br><<br>2<br>E<br><!-- End of picture text -->



<!-- Start of picture text -->
ES_<br>Es 4<br>2 HA Hat<br><!-- End of picture text -->

**(a) (b)** 

**Figure 4.23:** (a) Input Current and (b) Output Voltage waveforms at steady state with D= _D_ nom. 

#### **4.6 Comparative Performance** 

Following the theoretical analysis discussed in (3.2.1), this section presents the converter’s performance across different phase configurations. To show the practical differences within a variable range of interleaved stages for this specific application, a comparative analysis is conducted based on the simulation on the real components including the parasitic effects of the wires and contacts developed in Section 4.5. 

The evaluation focuses on three primary performance metrics: overall system efficiency, input current ripple, and output voltage ripple, alongside the actual RMS current stresses imposed on the physical components. 

46 

_Design Choices and Component Selection_ 



<!-- Start of picture text -->
aro<br>ois<br>" Z<br>= 06 Ni<br>a<br>Bos 049<br>oo : Oe<br>Conventional<br>BC 2 Phases IBC 3 PhasesIBC 4 PhasesIBC<br><!-- End of picture text -->



<!-- Start of picture text -->
a 708 oh<br>><br>06 N %<br>Es Ox<br>a ox<br>ooConventionalBC 2:  Phases IBC: 3 PhasesIBC—4 PhasesIBC<br><!-- End of picture text -->



<!-- Start of picture text -->
(a) (b)<br><!-- End of picture text -->

**Figure 4.24:** Input Current Ripple (a) and Output Voltage Ripple (b) comparison. 

Figure 4.24 highlights the impact of the interleaved configuration. In particular, 4.24a illustrates the input current ripple behavior when the phase inductance is held constant across an increasing number of stages. By keeping the current ripple constant rather than progressively reducing it, this configuration lets the designer reach smaller magnetic volumes, as clearly highlighted in Table 3.1. 

The unexpected behavior of ∆ _VO_ shown in 4.24b is primarily attributed to parasitic inductance spikes and the SPICE MEAS directives employed, which rely on the maximum and minimum trace values within a specific time interval. 



<!-- Start of picture text -->
— obo Be<br>Zh we onc<br>= [ae 4c<br>Eos<br>Z 06<br>3<br>a4<br>g<br>Zo<br>0.0 040 048 050 058 0 068 070 O78 O80<br>Duty Cycle (D)<br><!-- End of picture text -->



<!-- Start of picture text -->
we Be<br>jf -B 2Bc<br>= Sf snc<br>=<br>é<br>Z4<br>3<br>3<br>o ‘040048080 055 060068 070 075 080<br>Duty Cycle (D)<br><!-- End of picture text -->



<!-- Start of picture text -->
(a) (b)<br><!-- End of picture text -->

**Figure 4.25:** Output Capacitor Stack RMS current (a) and Switch RMS current (b) against Duty Cycle. 

Finally, the reduction in component stress achieved across different numbers of interleaved stages is represented in Figure 4.25. 

A slight increase in efficiency can be observed in Fig. 4.26a, likely attributable to reduced component stress. Meanwhile, Fig. 4.26b shows that while the conventional boost converter and the two-phase IBC topology behave almost identically;the fourphases configuration exhibits much more stable behavior across the analyzed duty cycle range, despite the relatively small variation of the CCM conversion ratio. 

47 

_Design Choices and Component Selection_ 



<!-- Start of picture text -->
wk ssi — B64 IBF<br>> 00<br>= 06 Vy) ox od<br>“ Oo.<br>K > vie<br>a2 + °<br>y “iod<br>bo S<br>“ConventionalBC 2 Phases IBC 3 PhasesIBC 4 Phases IBC<br><!-- End of picture text -->



<!-- Start of picture text -->
3216<br>32u4<br>€<br>= 3210<br>oes<br>B 3206<br>soa7 =te 210041BC<br>3.200 7) a8 06 a7 a8<br>Duty Cycle (D)<br><!-- End of picture text -->



<!-- Start of picture text -->
(a) (b)<br><!-- End of picture text -->

**Figure 4.26:** Efficiency _η_ (a) and Conversion Ratio _MCCM_ (b) comparison over a restricted range of duty cycles. 

The usage of an Interleaved stage must be weighted against other real variables such as the total system cost, including hidden expenses as the gate drivers for the additional phases and the related conditioning circuitry to properly control the converter. Today, the 2-phases and 3-phases IBCs are the most adopted solutions by the industry due to their optimal tradeoff between performance and cost balance. 

48 

### **Chapter 5** 

# **Control Techniques** 

Before introducing the digital control scheme and its practical implementation, a brief overview of analog control techniques is discussed. Specifically, Average Current Mode Control (ACMC) is analyzed, as it is widely adopted due to its advantages over traditional peak current control in terms of bandwidth, stability and noise immunity. 

Furthermore, since the multi-phase topology intrinsically involves parallel branches, active current sharing principles are explained, along with how they operate to equalize the current flow across all phases and prevent individual component overload. 

Single-sensor current control schemes are briefly analysed to evaluate the potential for overall system cost reduction by eliminating dedicated sensors on each leg. 

Digital control is primarily selected because it can fulfill the same functional requirements as analog schemes, with the main distinction being the application of a discrete-time transformation to operate properly within the digital domain. 

#### **5.1 Average Current Mode Control (ACMC)** 

The above mentioned control technique is often preferred over classical ones, such as Peak Current Mode Control (PCMC), due to its higher bandwidth and superior immunity to switching noise, which can otherwise induce abrupt duty ratio variations within a switching cycle. Furthermore, PCMC suffers from subharmonic instability when _D >_ 0 _._ 5, leading to oscillatory behavior that degrades overall system performance. 

To extract the fundamental component from the current waveform, a low-pass filter is typically applied downstream of the sensor. In this setup, the filtering action is inherently performed by the Hall-effect current sensor, eliminating the need for a bulky filter at the compensator input. 

49 

_Control Techniques_ 



<!-- Start of picture text -->
Kor<br>Koz<br><!-- End of picture text -->

**Figure 5.1:** Block diagram representation of ACMC for 2-phases IBC. [19] 

Where: 

- 1 _/Vm_ : Modulator gain 

- _KiD_ 1 _, KiD_ 2 _, Kv_ : Sensing gains 

- _Hv_ ( _s_ ) : Voltage loop compensator 

- _HiL_ 1( _s_ ) _, HiL_ 2( _s_ ) : Current loop compensators 

- _ZO_ ( _s_ ) : Output Impedance 

Figure 5.1 illustrates the complete control scheme for a two-phase IBC where the inductor current serves as the controlled variable. The inner current loop must first be analysed. Once its dynamic behavior is defined, the outer voltage loop is evaluated using the results obtained from the inner loop. Finally, the stability criteria can be ensured once the open loop gain _Tc,i_ ( _s_ ) transfer function is calculated. 



<!-- Start of picture text -->
Cc,<br>R,  G<br>R,<br>V(t Ry<br>a ane =a<br><!-- End of picture text -->

**Figure 5.2:** Type-2 Analog Compensator. [13] 

##### **5.1.1 Inner Current Loop** 

If the diode current is sensed instead of the inductor current, the derivation of the transfer function _GioD_ , which can be found in A.4, differs from conventional expressions. 

50 

_Control Techniques_ 



In fact, as can be observed in (5.1), the transfer function _GioD_ ( _s_ ) contains a righthalf-plane zero (RHPZ), the control loop is more challenging to design compared to inductor current sensing, which does not exhibit an RHPZ (see A.3). Consequently, this characteristic requires a lower-frequency pole, ultimately restricting the controller bandwidth. Because an RHPZ in the outer voltage loop is inevitable, despite the initial request, the inductor current will be sensed instead of the output one due to the limited system bandwidth and the necessity of a complex Type-3 compensator. 



In order to compare the two choices, a comparative analysis is performed to practically demonstrate that the choice of a inductor current control loop is way more practical than choosing the diode option. To solve this issue, a Type-2 compensator is implemented, featuring a pole at the origin to ensure zero steady-state error, one left-half-plane zero (LHPZ), and one left-half-plane pole (LHPP), as expressed by the following relation: 



Finally the open loop gain transfer function will be useful to determine the gain and phase margin to assess stability criteria. 



where _Hsense,i_ ( _s_ ) is the current sensor transfer function. 

##### **5.1.2 Outer Voltage Loop** 

Once defined the inner current loop, the system transfer function is obtained as: 



51 

_Control Techniques_ 

_vo_ <u>(</u> _s_ <u>)</u> Consequently, _iref_ ( _s_ )<sup>canbeextractedfrom:</sup> 



Where the three terms are evaluated as follows: 

Since the voltage loop needs to be designed at much lower frequencies than the _TiLx_ <u>(</u> _s_ <u>)</u> current loop, the term 1+ _TiLx_ ( _s_ )<sup>_≈_1,whiletheonlyfrequenciestakenintoaccount</sup> are _ωRHPZ_ and _ωp_ 1, which can be compensated by a Type-2 compensator as seen in (5.3). 

#### **5.2 Digital Control** 

Introducing digital schemes inside a pure analog circuitry can be a tough process, because moving the existing barrier closer to the digital domain increases the complexity of the control action. Moreover, stability criteria must be evaluated in the z-domain, while additional issues arise, such as bandwidth limitations, time and voltage measurement resolution constraints, and limit-cycle oscillations. 

52 

_Control Techniques_ 



<!-- Start of picture text -->
TT Voce Io transduced<br>ts AAR Signal<br>a a Conditioning<br>lo im<br>r es V<br>“Voc —<br>H abits ——— tt<br>{| Digital Control fi ov it<br>t PWM Algorithm rls a a rt<br>H A/D Conversion |<br>: (Ideal sampler + Quantizer) |<br>{Microcontrolleror DSP<br>Discrete time<br><!-- End of picture text -->

**Figure 5.3:** Typical organization of a digital current controller. [20] 

High-frequency converters must account for the internal processing delays of the microcontroller or DSP, ensuring that the entire sampling chain completes its operation within the given timeframe to prevent degradation of the control loop’s frequency response and bandwidth. 

Moreover, the presence of a signal conditioning block is mandatory to minimize the quantization error, which can lead to controller instabilities. Another issue that must be taken into account is related to the aliasing phenomenon, strictly connected to the Nyquist frequency for which, the sampling frequency must be at least 2 _·fmax_ (Fig.5.4). In most cases, this constraint cannot be satisfied due to hardware limitations. 

To solve this issue, the sampling and the switching process must be properly synchronized to exploit the geometrical relationship between the linear behaviour of the converter’s waveforms and the switching interval in order to get the average value, if this condition is not satisfied and a mismatch is present between the sampling and the switching frequency, unwanted low-frequency components will be generated at the output. 

The quantization process can be described by means of a stochastic process with a statistical power equal to: 



To describe the performance of the given quantizer in quantitative terms, the signalto-noise ratio definition can be used in order to determine the effective number of bits, or the equivalent bits of the control chain including all the possible errors within the system: 



53 

_Control Techniques_ 



<!-- Start of picture text -->
s(t) originaliginal signalsi am. spectrum™ replicasp  (due to sampling)pling}<br>aa Ane / A kA<br>N! t ‘\ Ne 2fc f<br>{ ~ \ “original signal spectrum<br>chnpe  Feconstructed signal aitaagerrer<br>aliasing = affected spectrumsa S(f) replicas<br>‘tT original spectrum “\<br>fel fc 2 me<br>we Nyquist frequency<br><!-- End of picture text -->

**Figure 5.4:** (Top-left) effect of a too low sampling frequency on the reconstructed signal; (Top-right) frequency domain interpretation of the aliasing phenomenon; (Bottom) partial overlap of the spectrum spectrum replicas due to aliasing.[20] 

##### **5.2.1 Digital PWM (DPWM)** 

In order to provide a useful representation in the digital domain, the continuous-time blocks encountered when approaching to the analog compensator will be converted into discrete-time blocks. 



<!-- Start of picture text -->
1 Toner er | mil ida) o--—— |=<br>{ H z-1 H i ot i<br>i Be) | 7] A<br>Microcontroller or DSP<br><!-- End of picture text -->

**Figure 5.5:** Block diagram of the digital current control loop with PI regulator. [20] 

In particular, each analog block will be translated into its digital equivalent, which have the same objective of the analog one but considering the limited resolution and related effects as the non null delay. 



The relation shown in (5.6) can be used to determine the minimum number of bits required to represent the duty cycle. Where _f_ clock is the frequency of the binary counter and _fs_ the desired switching frequency. Typically, _Ne_ is lower than the 

54 

_Control Techniques_ 

number of bits _n_ available inside the counter of the microcontroller or DSP employed. 



<!-- Start of picture text -->
wool oo<br>2th, He<br><!-- End of picture text -->



<!-- Start of picture text -->
Bir Comer Tint nat<br>i fi H Programmed duty-cycle<br>Ts ‘<br><!-- End of picture text -->



<!-- Start of picture text -->
(a) (b)<br><!-- End of picture text -->

**Figure 5.6:** Analog (a) and Digital (b) PWM 



<!-- Start of picture text -->
°<br>Ts °<br>ctomey tmey emo} ap<br>= eH) Y= mit) CSAS tty amity<br>ISIN<br>7 t en ar = p<br>Veit) Vuelt) : Yaoi i<br>b) °) d<br><!-- End of picture text -->

**Figure 5.7:** Uniformly sampled PWM with single update mode: (a) general block diagram of Zero-Order-Hold sampling model, (b) trailing-edge modulation, (c) leadingedge modulation, (d) triangular carrier modulation. [20] 

Depending on the carrier signal _c_ ( _t_ ) selected, different modulation schemes are obtained. The trailing-edge carrier is considered in the subsequent chapters, characterized by the following expression: 



where _c_ PK is the carrier peak, _V_ MO( _s_ ) and _M_ ( _s_ ) represent the Laplace transforms of the modulated signal _V_ MO( _t_ ) and the carrier _m_ ( _t_ ). The relation (5.7) shows in quantitative terms that the delay introduced by the modulator is proportional to the duty cycle, as illustrated in Fig. 5.7. For this reason, introducing multiple update modulation is useful to reduce at scale the amount of delay inside the feedback loop, which as a matter of fact, decreases the phase margin and forces a reduction of the control-loop bandwidth thus slowing the transient response. For the sake of curiosity, from the (5.8) it’s possible to acknowledge the delay reduction by looking at the 

55 

_Control Techniques_ 

transfer function related to multiple update modulation: 



##### **5.2.2 Digital Compensator Modeling** 

To derive the discrete-time blocks, several discretization strategies can be applied. Every time discretization is applied to a system, it will exhibit a performance degradation due to the loss of information. The basic concept behind the strategies is the substitution of the continuous-time waveforms into a suitable numerical approximation which can be easily integrated. 



<!-- Start of picture text -->
Backward<br>Euler<br>[TU ForwardEuler<br>(k-1)Ts Ts (k+I)Ts t (k-1)Ts. kTs (k+1)Ts t<br>a) b)<br><!-- End of picture text -->

**Figure 5.8:** (a) Euler integration method (forward and backward). (b) Trapezoidal integration method (Tustin approximation). [20] 

Then, the sum of the areas must be translated from the s-domain to the Z-domain using the relationship between the area expression and the samples, by means of a recursive function. The latter relationship, depending on the type approximation applied, leads to a different equivalence between the continuous _s_ and the discrete _z_ variables. 

|**METHOD**|**Z-FORM**|**3% DISTORTION LIMIT**|
|---|---|---|
|Backward Euler<br>Forward Euler<br>Trapezoidal (Tustin)|_s_= <sup>_z−_1</sup><br>_z·TS_<br>_s_= <sup>_z−_1</sup><br>_TS_<br>_s_=<br>2<br>_TS_<br>_z−_1<br>_z_+1|_fs_<br>_f _<sup>_>_ 20</sup><br>_fs_<br>_f _<sup>_>_ 20</sup><br>_fs_<br>_f _<sup>_>_ 10</sup>|



**Table 5.1:** Discretization Methods. [20] 

Tustin approximation will be exploited due to its wider frequency range validity with respect to the Euler methods. 

##### **5.2.3 Limit-Cycle Oscillations (LCO)** 

Arithmetic quantization and output quantization are related to the finite precision and the truncation operated by the fixed-point unit inside the DSP or microcontroller. Today, floating-point units are widely available inside commercial products letting the 

56 

_Control Techniques_ 

user minimize these kind of errors. However, the need of minimizing the computational delay forces the designer to choose fixed-point architectures and so, adapt to the instability and distortion of the controller’s frequency response, which have unevitably an effect on the overall system performance. 



<!-- Start of picture text -->
dx<br>“[au]<br>0.500 |---desired setpoint 4<br>27! ext | 3<br>0.250 4 { " I a les<br>0.125 of aie eo *<br>-7] 001 010 O11 100 dy , eee t<br>. Tico<br><!-- End of picture text -->

**Figure 5.9:** Limit Cycle Oscillation due to invalid set-point among duty-cycle possible outputs. [20] 

In Fig. 5.9 the limit-cycle oscillations (LCOs) phenomenon is represented, it causes continuous switching between two discrete states, leading to unwanted oscillations in the steady-state current. To mitigate this phenomenon, the following conditions must be satisfied: 



where _Gdc_ is steady-state gain of the transfer function between the controlled variable and the duty cycle, _q_ DPWM and _q_ ADC the duty cycle and ADC resolution respectively. If the duty cycle resolution is coarser than the ADC resolution, each correction step induces an output swing larger than the quantization step causing the oscillatory behaviour. The second condition presented in (5.9) specifies the integral action required to counteract the proportional (P) or proportional-derivative (PD) control action, given by _Gdc · KP · q_ ADC. 

Satisfying these conditions is necessary to theoretically eliminate LCOs; however, in practice, non-linear effects can still be induced by the quantizer within the feedback loop. Additionally, a third condition requires the closed-loop describing function to be stable; but this criterion applies only to limit-cycle oscillations that can be accurately represented by sinusoidal waveforms [20][21][22]. 

#### **5.3 Controller Design** 

From the _Gvd_ ( _s_ ) and _Gid_ ( _s_ ) the following informations can be obtained neglecting the effect of the capacitor ESR: 



57 

_Control Techniques_ 



In order to select a proper value for the crossover frequency, a valid margin to comply with the Shannon’s criteria is to select _fc ≈_<sup>_<u>f</u>_</sup> 10<sup><u>sw</u>.SincetheRHPZmustbetakeninto</sup> account in the crossover frequency selection, _fc_ is selected as: 



The maximum possible value for _fc,i_ is selected to maximize the speed of the inner control loop. 

##### **5.3.1 Inner Current Loop** 

With this procedure is possible to determine the position of the poles and zeros of the compensator in order to achieve the desired crossover frequency and phase margin. A phase margin of 60<sup>_◦_</sup> is selected as it’s considered an industry standard. 



**Figure 5.10:** _Gid_ ( _s_ ) frequency response in MATLAB. 

###### **5.3.1.1 K-factor Method** 

To design the compensator for the inner current loop, the magnitude and phase of the _Gid_ ( _s_ ) shown in Fig. 5.10 can be extracted. 



Then, the required phase increase that must be introduced by the compensator is evaluated as: 



58 

_Control Techniques_ 

So, the K factor can be calculated: 



Consequently, the position of the compensator poles and zeros is fixed. 



Finally, the DC gain of the compensator _Kc,i_ : 



Where _|H_ sense,i( _s_ ) _|_ is the gain of the sensor block including the conditioning circuitry while _Vm_ is the amplitude of the PWM ramp. 



<!-- Start of picture text -->
Bode Diagram - Compensator Ge!<br>2<br>Frequency (rads)<br><!-- End of picture text -->



<!-- Start of picture text -->
Bode Diagram<br>Gm = 26.1 dB (at 1.75e+05 radis), Pm = 60 deg (at 2.20404 rad/s)<br>2<br>Frequency (radis)<br><!-- End of picture text -->



<!-- Start of picture text -->
(a) (b)<br><!-- End of picture text -->

**Figure 5.11:** Bode plots in MATLAB of (a) Compensator and (b) Inner Current Loop gain transfer functions. 

##### **5.3.2 Outer Voltage Loop** 

Regarding the outer loop, the same procedure is applied acknowledging that in this case, the crossover frequency must be much lower than the inner loop one. As a rule of thumb, the crossover frequency of the voltage loop is selected as _fc,v ≈_<sup>_<u>fc</u>_</sup> 5<sup>_<u>,i</u>≈_600Hz.</sup> The position of the poles is then fixed to achieve direct pole-zero cancellation. In this case, it’s possible to evaluate the gain of the voltage controller as: 



59 

_Control Techniques_ 

Where _H_ sense,v( _s_ ) is the output voltage sensor transfer function and for the sake of simplicity is described by its DC gain _Hv_ 0, set in order to achieve 3V at the ADC input considering the maximum output voltage (50.4 V). Finally, _KiLx_ is the gain of inner loop extracted in the previous section. 

##### **5.3.3 Analog Compensator** 

Given the desired pole and zero frequencies in (5.11), the components of the Op-amp based compensator are derived as follows: 



##### **5.3.4 Digital Compensator** 

As explained in section (5.2.2), the idea is to transform the continuous-time compensator transfer function into a discrete-time one by using a discretization method, in this case the Tustin approximation will be exploited to obtain the function in the Z-domain in order to be able to implement it in the microcontroller. To obtain the discrete form in the z-domain, we need to apply the bilinear transformation in (5.13): 



Where _Ts_ is the sampling period of the digital controller. For the sake of completeness in (5.14) is shown how a typical continuous time factor is transformed using the Tustin transformation. 



After applying (5.14) to the compensator transfer function (5.3), it’s possible to obtain a recursive difference equation which can be implemented inside a DSP or microcontroller using the _z_<sup>_−_1</sup> form of the s parameter. Since this operation has several stages of computation, the value of the coefficients shown in (5.15) can be evaluated using a pre-built function. The standard Tustin approximation is valid for frequencies below _fs/_ 2, if the poles in the function exceed this limit this effect should be considered as it cause a downward shifting of the poles. Since the frequency of the poles is a fraction of the switching frequency, the standard approximation is valid. 

**Listing 5.1:** Tustin bilinear transformation implemented in MATLAB ® 

1 <mark>s = tf( ’s’);</mark> 

60 

_Control Techniques_ 

- 2 <mark>G_cv_s = (K_c * (1 + s/w_z)) / (s * (1 + s/w_p));</mark> 

- 3 <mark>G_cv_z = c2d(G_cv_s , Ts, ’tustin ’);</mark> 

- 4 <mark>% Extract [b0, b1, b2] and [1, a1, a2] vectors</mark> 

- 5 <mark>[b, a] = tfdata(G_cv_z , ’v’);</mark> 



Where _x_ [ _k_ ] _, x_ [ _k −_ 1] _, .._ and _y_ [ _k_ ] _, y_ [ _k −_ 1] _, .._ values are stored inside a buffer and represent the past errors and the related output at instant _t_ = _k, t_ = _k −_ 1 _, .._ 



<!-- Start of picture text -->
bo<br>a(n) [> (+) y(n)<br>by —a E<br>[> <]<br>A by ay E<br><J<br><!-- End of picture text -->

**Figure 5.12:** FIR filter followed by recursion. 

Today, most commercial DSPs feature floating-point units (FPUs), which provide precise and reliable results when performing floating-point operations. However, to save computation time, fixed-point arithmetic is used to push the bandwidth to its maximum, at the cost of losing precision due to truncation. 

#### **5.4 Possible implementations** 

When operating multiple stages or branches in parallel, manufacturing tolerances and component mismatches across the channels can lead to current imbalances, causing drifts and degradation of the overall converter. The ACMC/PCMC control topology lacks a differential loop to measure phase current error ( _iLi − iLi±_ 1). The phases operate independently without knowing whether another phase is carrying more or less load. 

To mitigate this issue, Current Sharing control techniques are employed to automatically equalize the current distribution among the phases, thereby balancing the stresses. 

Depending on the control architecture, current sharing can be implemented through passive or active techniques. Passive methods, most notably Droop Control, introduce an artificial impedance (such as a virtual resistance) into the voltage regulation loop; this causes a deliberate, proportional drop in output voltage as current increases, 

61 

_Control Techniques_ 

naturally forcing parallel branches toward equilibrium without requiring dedicated communication buses. On the other hand, active current sharing strategies, such as the Master-Slave and the Democratic approach rely on dedicated feedback loops to enforce precise dynamic balancing. 

While various passive and active strategies exist in the literature, the following sections focus specifically on the Droop, Democratic and Master-Slave control approaches. 

##### **5.4.1 Droop Method** 

The droop method, often referred as autonomous or wireless control, is a fundamental technique for managing power sharing among parallel converters. Its operating principle relies on localized self-regulation: if a specific branch begins to output excessive current due to a localized imbalance, its internal voltage undergoes a controlled, proportional drop. 



This intentional reduction forces the load current to naturally redistribute across the other parallel branches. Consequently, the output impedance effectively functions as an active control variable within the system. 



<!-- Start of picture text -->
V9" a oe y<br>Virtual output impedance loop<br><!-- End of picture text -->

**Figure 5.13:** Block diagram of the closed-loop system with the virtual output impedance path. 

Nevertheless, the droop method exhibits several drawbacks that limit its application scope. These include a slow transient response, an inherent tradeoff between powersharing accuracy and allowable voltage or frequency deviations, unbalanced harmonic current distribution, and a strong dependency on the output impedance characteristics of the inverter. 

##### **5.4.2 Active sharing approaches** 

Active current-sharing control strategies provide superior current balancing performance by utilizing a common current reference. This shared reference allows the behavior of individual stages to be equalized through the approaches outlined below. 

62 

_Control Techniques_ 

###### **5.4.2.1 Master-Slave** 

Diverse current-sharing strategies can be implemented using dedicated (Fig. 5.14), rotating, or automatic (Fig. 5.15) master-slave configurations. In a dedicated master architecture, the master unit is fixed at the hardware level, introducing a single point of failure that halts the entire system if the master unit degrades or fails. 

To overcome this limitation, automatic or rotating master schemes, such as the Highest-Current Master-Slave method, dynamically assign the master role on the bus to the module supplying the highest reference current. In these configurations, slave modules are regulated via the current-sharing bus and a closed-loop controller, allowing their output currents to track the reference current much more accurately compared to droop control. However, this topology remains sensitive to interconnect failures: if the current-sharing bus or the central controller fails, the reliability and operational functionality of the entire system are completely destroyed. 





**Figure 5.15:** Highest-Current Masterslave current sharing control strategy. 

**Figure 5.14:** Fixed Master-slave current sharing control strategy. 

###### **5.4.2.2 Democratic** 

This method addresses the reliability limitations associated with the master-slave configuration by allowing the converter to maintain operation even if one of the branches fails. Each module retains its inner current and outer voltage loops, while the individual output currents _Ik_ are injected through appropriately weighted resistors _Rs_ . Applying Millman’s theorem, the shared bus current corresponds to the average of the output currents from all modules, denoted as _Iavg_ . Finally, a differential amplifier evaluates the error ∆ _Ik_ = _Iavg − Ik_ to adjust the reference voltage of each module. 

63 

_Control Techniques_ 

##### **5.4.3 Single-Sensor Current Sensing** 

Implementing multi-stage structures requires a large number of sensors and complex control networks, which reduces the available usable volume. Single-sensor schemes were developed to lower hardware costs associated with the sensing apparatus by integrating estimation techniques directly into the control loop. 

In the interleaved boost converter topology, a single current sensor can be used instead of one per branch by sampling the total diode current, which represents the sum of all phase currents and subsequently the estimation can be performed, as illustrated in Fig. 5.16a. However, when parasitic inductances from contacts or cables and stray capacitances due to trace proximity are taken into account, unwanted resonance phenomena can occur, as highlighted in Fig. 5.16b. For this reason, employing digital architectures, the user can control precisely the sampling interval in order to avoid those parasitic peaks, which can lead to a measurement error.[23][24] 



**(a)** 



**(b)** 

**Figure 5.16:** (a) Two-Phase Interleaved Boost Converter with Single Sensor based Current Balancing Scheme. (b) Noise at the Switching Instances of Inductor Current. [23] 

The key estimation techniques include: 

- **Geometrical Approach** : The DSP exploits the geometric relationships among the IBC phases to reconstruct individual phase currents based on the linear behavior of the inductor current. 

- **Model-Based Estimation** : Predictive algorithms based on previous measurements minimize tracking error relative to the applied duty cycle in real time, thereby reducing overall system cost. This approach requires a detailed mathematical model of the converter to execute the control action. 

64 

### **Chapter 6** 

# **PCB Implementation** 

The design process has been carried out using Altium Designer ®. It was used due to its wide adoption in the industry and strong software capabilities. The project is developed around a hierarchical structure explained in the following section. 

#### **6.1 Floor Planning** 



**Figure 6.1:** Block disposition inside the board. 

As highlighted in Fig. 6.1, four leaves are included in order to achieve different operating conditions represented in Fig. 6.2. SuppliesLV is the block containing the buck converters which supply all the internal blocks and the reference voltage for the current conditioning circuitry. While Leaf is the elementary boost cell which is parallelised. 

65 

_PCB Implementation_ 



<!-- Start of picture text -->
ne<br>ri ba U<br>Bay<br><!-- End of picture text -->



<!-- Start of picture text -->
ho<br>= < 4i<br><<br><!-- End of picture text -->



<!-- Start of picture text -->
Phe<br>ho < i:oS<br>@.<br><!-- End of picture text -->



<!-- Start of picture text -->
(a) (b) (c)<br><!-- End of picture text -->

**Figure 6.2:** System operated at (a) four sources in parallel, no interleaving operation, (b) two sources in parallel using two interleaved phases. (c) single source with four interleaved phases. 

#### **6.2 STM32-F334R8 Microcontroller** 

The STM32F334R8 is typically used in digital power supplies and lighting applications primarily for its High-Resolution Timer (HRTIM) peripheral, capable of generating up to twelve signal with a time resolution of 217 ps. Because the minimum time step is strictly related to control action stability, avoiding limit-cycle oscillations requires providing a fine duty-cycle resolution by selecting the smallest time step available. Section 5.2.3 provides a theoretical evaluation of the minimum resolution required to avoid limit-cycle oscillations. [25] 

**Table 6.1:** Phase Pinout and Channel Mapping 

|**Phase**|**Curre**|**nt**|**Voltag**|**e**|**PWM**||
|---|---|---|---|---|---|---|
|1|ADC2_|IN1|(PA4)<br>ADC1|_IN6 (PC0)|HRTIM1_|CHA1 (PA8)|
|2|ADC2_|IN3|(PA6)<br>ADC1|_IN7 (PC1)|HRTIM1_|CHB1 (PA10)|
|3|ADC2_|IN4|(PA7)<br>ADC1|_IN8 (PC2)|HRTIM1_|CHC1 (PB12)|
|4|ADC2_|IN5|(PC4)<br>ADC1|_IN9 (PC3)|HRTIM1_|CHD1 (PB14)|



Lastly, the output voltage was assigned to ADC2_IN11 (PC5), the output current to ADC1_IN12 (PB1) and the temperature sensing signal to ADC2_IN15 (PB15). 

#### **6.3 Power Blocks Schematics** 

##### **6.3.1 Power Stage** 

Starting from the top of the hierarchical structure, I have placed four sheet symbols corresponding to the leaves in parallel. The output of each leaf is then fed to the output capacitor stack consisting of multiple electrolytic and ceramic capacitors. C1, C2, and C3 are 100V-rated electrolytic capacitors used to distribute the RMS current stress. C4, C5, and C6 are introduced to flatten the frequency response, as shown in (4.2.5). D1 is a TVS diode placed close to the output terminal for ESD protection. The output current is measured by placing the Hall effect sensor between the output net LIPO and the net CO_STACK_IN. Both the INA326 and the TLV9151 feature 1 µF and 100 nF, 16V-rated decoupling capacitors to stabilize the supply voltage, alongside output RC filters to mitigate crosstalk from nearby traces. 

66 

_PCB Implementation_ 

Regarding the LiPo stack voltage conditioning circuitry, a shunt resistor is placed in parallel with the potentiometer to fix the resistor divider ratio when the potentiometer is no longer necessary. Additionally, the resistances in the divider branch were selected to ensure negligible current draw. 

Finally, the Hall effect conditioning circuitry includes a potentiometer with a series resistance to adjust the gain, while capacitor C7 is chosen to set the bandwidth to 50 kHz. 



<!-- Start of picture text -->
sails ead omen (SEE,<br>| me EE CS cai.<br>ae LiPo stack Voltage Conditioning circuitry<br>Wall Effect Sensor Conditioning Clreuttry = ; .<br>_a<br><!-- End of picture text -->

**Figure 6.3:** Power Stage schematic. 

##### **6.3.2 Leaf** 

The leaf section incorporates the basic boost converter topology along with a few additional features to comply with the worst-case scenario where four leaves operate in parallel. From the top-left, the D5 TVS diode is placed close to the input connector to mitigate ESD phenomena. The D2 Schottky diode is used to prevent reverse polarity. Ceramic capacitors C18, C19, C20, and C21 are not crucial but are needed to limit EMI issues. The same considerations made in the previous section apply to the leaf voltage and phase current conditioning circuitry. 

The IR2110S gate driver has two supplies: the output side is supplied with 12V, while the logic side is supplied with 3.3V. Both sides include decoupling capacitors for stabilization, and unused terminals are tied to ground to prevent any unwanted switching behavior caused by floating pins. **LEAF_PWM** is the PWM signal coming from the microcontroller. Before connecting the PWM signal to the **LIN** terminal, since it involves a 400 kHz square wave traveling along a long copper trace, an RC filter is required to preserve signal integrity. 

A damping resistor with an anti-parallel diode is placed together between the driver output and the gate terminal. A shunt resistor is also included to subsequently 

67 

_PCB Implementation_ 

evaluate its effect on switching times. 

Finally, an RC snubber network is placed at the output side of the MOSFET to damp natural oscillations during the switching intervals. 



<!-- Start of picture text -->
TeSnSe |<br>hal ‘& wall Effect Sensor - Phase Current<br>uw (ened man) Te [Poem<br><!-- End of picture text -->

**Figure 6.4:** Leaf schematic. 

##### **6.3.3 SuppliesLV** 

The LMR36510 buck converters are cascaded in order to step down the LiPo stack voltage. In addition, they are coupled with external connectors and physical jumpers to allow bypassing the buck converters during the debugging phase. The first buck stage can achieve an efficiency of around 60%, according to the manufacturer’s datasheet, due to the large gap between the input and output voltages. Since the current drawn does not exceed 500 mA, a dedicated heatsink could be useful, though not strictly necessary. 

Then, due to space constraints and to maintain a functional hierarchy, the 2.4V voltage reference is placed in this schematic, featuring a variable resistor divider followed by a buffer. 

68 

_PCB Implementation_ 



**Figure 6.5:** Low-Voltage supplies schematic. 

#### **6.4 Trace Width** 

The SaturnPCB toolkit® was employed to evaluate the physical requirements of the power traces, taking into account the maximum DC current flowing at the input under worst-case conditions, which is determined by the maximum current rating of the input FC (8 A). After configuring the basic settings for an FR4 substrate with the corresponding PCB stackup and maximum temperature increase, the minimum conductor width is 25.4 mm. 



**Figure 6.6:** Trace width evaluated through SaturnPCB toolkit ®. 

69 

_PCB Implementation_ 

#### **6.5 Layout** 

To ensure proper thermal management during normal operation while mitigating unwanted effects such as EMI, the prototype was developed using a four-layer stackup. Thanks to the internal ground and power planes, components can be placed closer together to further reduce parasitic impedance. 







<!-- Start of picture text -->
(a) Top Layer. (b) Ground Plane.<br>(c) Power Plane. (d) Bottom Layer.<br><!-- End of picture text -->

**Figure 6.7:** PCB layers. 

Both the switching components and their gate drivers are placed on the bottom layer to connect directly with the heatsink, enhancing the mechanical stability. Since the output termination, conditioning circuitry, and output current sensor are located on the top layer, a group of vias is needed to connect the output of the leaves to the output capacitor stack. The external supply terminals for 12 V and 5 V are located in the top-right corner. 

Polygon pours are employed both in top and bottom layers in high power traces to minimize the ohmic losses and maximize the thermal dissipation. 

#### **6.6 Cost Analysis** 

To evaluate the cost of the project, three macro areas should typically be considered: the cost of materials, including electronic and mechanical components as well as the cost of production of the PCB, which is manufactured by Safe-PCB ®; labour 

70 

_PCB Implementation_ 



<!-- Start of picture text -->
ge Se Sm Te<br>ae) J a eens eee<br><!-- End of picture text -->



<!-- Start of picture text -->
a.<br>“, ae<br><!-- End of picture text -->



<!-- Start of picture text -->
(a) Top (b) Bottom<br><!-- End of picture text -->

**Figure 6.8:** Board 3D Model. 

cost, that takes into account the time spent to analyse, design, build and test the prototype; cost of services, which is crucial when dealing with software licenses, laboratory equipment and experienced technicians to support the workflow. In our case, due to the lack of the latter information, the graph presented in Fig. 6.9 is based solely on the component costs, with DigiKey® as the primary supplier, and the PCB manufacturing costs from Safe-PCB®. 



<!-- Start of picture text -->
500,000€<br>450,000€<br>& 400,000 €<br>350,000<br>Zz 300,000 €<br>QSS 250,000€<br>8 200,000 €<br>la 150,000 €<br>3<br>Oo 100,000 €<br>50,000€<br>e €<br>Z 10 100 1000<br>Number of boards<br><!-- End of picture text -->

**Figure 6.9:** Unit price per board versus production volume. 

71 

### **Chapter 7** 

# **Conclusions and Future Works** 

In this study, a detailed analysis of the interleaved topology has been carried out, along with the design and implementation of the signal conditioning and power delivery blocks. 

The simulation results show the behaviour of the converter’s efficiency with an increasing number of phases, clearly demonstrating why two-phase and three-phase IBC structures are widely employed in the industry. The intricate interconnection between the STM32 peripherals, such as the High-Resolution Timer (HRTIM), Direct Memory Access (DMA), and the ADC has been investigated, along with evaluating the effects of high-level design choices on filter performance. 

However, a few practical limitations are still present. Currently, to fully exploit interleaved operation, the legs must be hardwired together manually. The initial objective of building a prototype as a solid base for future improvements must be carefully evaluated against its practical constraints. While achieving a 1 MHz switching frequency is fully feasible from the MCU side, it can introduce signal integrity and EMI issues on the converter side due to trace geometry. Additionally, the computational cost of the digital filter must be carefully analysed to avoid any performance degradation in the dynamic response. Several improvements can still be made, for example: 

- Verify the feasibility of increasing the switching frequency from 400 kHz to 1 MHz in terms of EMI, signal integrity, and control loop bandwidth. 

- Since the boost topology directly connects the source to the load through the phase inductor, built-in hardware short-circuit protection should be included to protect the main switching devices. 

- Despite the higher cost, the use of Litz wire in the custom inductors should be investigated to reduce the associated AC winding losses. 

- Analyze the possibility of using coupled inductors across the different phases 

72 

_Conclusions and Future Works_ 

to replace four separate cores with a single magnetic core, thereby reducing the board footprint. 

- To further reduce board size, a multi-layer layout stacking high-frequency switching elements over analog conditioning circuitry could be explored. 

- To minimize system size, fixed-resistor signal conditioning networks can replace bulky trim potentiometers. 

- To reduce overall cost, shunt resistors could be employed instead of Hall-effect sensors despite their higher power dissipation. In this case, the sensor placement must be carefully chosen to minimize losses associated with the phase RMS current. 

- Implement an automated leg configuration mechanism using dedicated relays or solid-state switches to eliminate manual hardwired jumpers. 

- From the firmware side, a solid Finite State Machine (FSM) can be implemented to perform the additional offset adjustment of the conditioning chain, along with the setup of the Look-Up table for the temperature measurements. 

The LTspice simulation files, MATLAB scripts, STM32 firmware projects, and Python plotting scripts developed throughout this project are available on GitHub at: https://github.com/davidectnia/InterleavedBoost-LIGH2TDRONE. 

73 

### **Appendix A** 

# **Transfer Functions Manipulation** 

#### **A.1 Interleaved Boost Converter Small-Signal model** 

For each leg _k_ = 1 _, ..., N_ , applying KVL across the inductor branch yields the following expression: 



The generic variable _X_<sup>ˆ</sup> represents the superposition of the steady-state DC value and the small-signal perturbation. Applying A.1, the currents sum up all the _N_ individual phases resulting in the total input current, _Iin_ =<sup>∑︁</sup><sup>_N_</sup> _k_ =1<sup>_ILk_obtainingthe</sup> relation in A.2. 



Dividing both sides by _N_ , we introduce the equivalent total inductance of the parallel system, _Leq_ =<sup>_L_</sup> _N_<sup>_<u>ph</u>_:</sup> 



This is the exact averaged inductor equation of a conventional single-phase Boost Converter but with the introduction of the equivalent total inductance term. 

For what concerns the output node, all the legs current sum up at the capacitor terminal: 



Since the duty cycles are equal between all the legs, the (1 _− d_ ) term can be factored out yielding the same relation found for the conventional Boost Converter. 

74 

_Transfer Functions Manipulation_ 



By injecting a small AC perturbation ( _x_ ) into the state variables, neglecting the second-order non-linear terms (e.g., _d · iin_ , _d · vout_ ) and separating the DC terms, the small-signal model can be extracted. 



Provided that the phases remain symmetrical and operate under identical duty _<u>Vout Vout</u>_ cycles, the control-to-output (<sup>ˆ︃</sup> _Vin_<sup>),alsodefinedasaudio-</sup> _d_ ˆ︁<sup>)andinput-to-output(ˆ︃</sup> ˆ︂ susceptibility, transfer functions of a generic N-phases interleaved boost converter (IBC) match those of a conventional boost converter, provided the phase inductance is replaced by an equivalent inductance _Leq_ =<sup>_L_</sup> _N_<sup>_<u>ph</u>_.</sup> 

To obtain the linear time-invariant model, Laplace transform is applied to perform linearization procedure after the averaging. The complete derivation of the transfer function is shown in A.2. 



Other types of transfer functions can be obtained as well by imposing the proper small signal conditions. In our case, since output current control scheme is employed, the definition of the current-to-output transfer function _GiD_ is shown in A.8 and complete computation is available in A.4. 



#### **A.2 Computation of Boost** _Gvd_ ( _s_ ) **using SSA [13]** 

From A.6, Laplace is applied to obtain 



75 

_Transfer Functions Manipulation_ 

Then solving for _v_ ˆ︁ _o_ ( _s_ ) : 



So: 

Finally: 



#### **A.3 Computation of Boost** _Gid_ ( _s_ ) 

ˆ _iL_ <u>(</u> _s_ <u>)</u> To derive the control-to-inductor current transfer function _Gid_ ( _s_ ) = _d_ ˆ( _s_ )<sup>,westart</sup> from the small-signal averaged state-space equations of the Boost converter. 

Assuming the input voltage perturbation is zero ( _v_ ˆ _in_ = 0), the equations for the inductor voltage and capacitor current are: 



First, we solve the second equation to isolate the output voltage _v_ ˆ _out_ : 



76 

_Transfer Functions Manipulation_ 



Next, we substitute this expression for _v_ ˆ _out_ into the first inductor equation: 



Multiplying the entire equation by (1 + _sRC_ ) to clear the denominator yields: 



Now, we group the<sup>ˆ</sup> _iL_ terms on the left side and the _d_<sup>ˆ</sup> terms on the right side: 



In a steady-state Boost converter, the DC inductor current is related to the output voltage by the load equation _IL_ = _<u>Vout</u>_ Substituting _IL_ into the right side _R_ (1 _−D_ )<sup>.</sup> simplifies the expression: 



Now, we can formulate the transfer function by dividing<sup>ˆ</sup> _iL_ by _d_<sup>ˆ</sup> : 



To achieve the canonical standard form, we divide the numerator and denominator by _R_ (1 _− D_ )<sup>2</sup> and substitute the steady-state voltage relation _Vout_ = 1 _<u>V−inDinD</u>_<sup>:</sup> 

_<u>V−inDinD</u>_ 1 _−inDD_<sup>:</sup> 



This leads to the final transfer function written in the standard Bode form, which allows for the immediate identification of the static gain, the zero, and the double pole: 



Where the characteristic parameters are defined as: 





77 

_Transfer Functions Manipulation_ 

#### **A.4 Computation of Boost** _GioD_ ( _s_ ) **[26]** 

By applying Kirchhoff’s Current Law (KCL) at the output node, the small-signal diode current<sup>ˆ</sup> _iD_ splits into the output capacitor current and the load resistor current: 



Expressing these currents in terms of the small-signal output voltage _v_ ˆ _out_ : 



Factoring out _v_ ˆ _out_ : 



Divide both sides of the equation by the duty cycle perturbation _d_<sup>ˆ</sup> : 



The canonical small-signal transfer function _Gvd_ ( _s_ ) for a Boost converter is shown in A.2 as: 



Multiplying this by the factor derived before yields: 



78 

# **Bibliography** 

- [1] U. Remme, “Global Hydrogen Review 2025,” 2025 (cit. on pp. 1–3). 

- [2] P. Halder et al., “Performance, emissions and economic analyses of hydrogen fuel cell vehicles,” _Renewable and Sustainable Energy Reviews_ , vol. 199, p. 114 543, Jul. 2024. Accessed: Aug. 6, 2026 (cit. on p. 2). 

- [3] European Commission. Joint Research Centre et al., _Clean Energy Technology Observatory, Fuel Cell Technology in the European Union_ (EUR). LU: Publications Office of the European Union, 2025. Accessed: Aug. 6, 2026 (cit. on pp. 3, 4). 

- [4] P. Thounthong and B. Davat, “Study of a multiphase interleaved step-up converter for fuel cell high power applications,” _Energy Conversion and Management_ , vol. 51, no. 4, pp. 826–832, Apr. 2010. Accessed: Aug. 6, 2026 (cit. on p. 4). 

- [5] S. Farhani, A. N’Diaye, A. Djerdir, and F. Bacha, “Design and practical study of three phase interleaved boost converter for fuel cell electric vehicle,” _Journal of Power Sources_ , vol. 479, p. 228 815, Dec. 2020. Accessed: Aug. 6, 2026 (cit. on pp. 4, 10). 

- [6] X. Hao, I. Salhi, S. Laghrouche, Y. Ait Amirat, and A. Djerdir, “Multiple inputs multi-phase interleaved boost converter for fuel cell systems applications,” _Renewable Energy_ , vol. 204, pp. 521–531, Mar. 2023. Accessed: Aug. 6, 2026 (cit. on pp. 4, 18, 19). 

- [7] S. M. Gheasaryan, E. Roshandel, and S. M. Saghaian-Nejad, “Proposing an Interleaved Boost Converter with Novel Soft-switching Approach in Photovoltaic Applications,” (cit. on pp. 5, 17, 31). 

- [8] X.-F. Cheng, C. Liu, D. Wang, and Y. Zhang, “State-of-the-Art Review on Soft-Switching Technologies for Non-Isolated DC-DC Converters,” _IEEE Access_ , vol. 9, pp. 119 235–119 249, 2021. Accessed: Aug. 6, 2026 (cit. on p. 5). 

- [9] H. Bodur and A. Bakan, “A new ZVT-PWM DC-DC converter,” _IEEE Trans. Power Electron._ , vol. 17, no. 1, pp. 40–47, Jan. 2002. Accessed: Aug. 6, 2026 (cit. on p. 5). 

- [10] V. A. s.r.o. “H-100 PEM Fuel Cell - 100W,” Accessed: Aug. 18, 2026 (cit. on p. 5). 

79 

_BIBLIOGRAPHY_ 

- [11] O. Hegazy, J. V. Mierlo, and P. Lataire, “Analysis, Modeling, and Implementation of a Multidevice Interleaved DC/DC Converter for Fuel Cell Hybrid Electric Vehicles,” _IEEE Trans. Power Electron._ , vol. 27, no. 11, pp. 4445–4458, Nov. 2012. Accessed: Aug. 6, 2026 (cit. on p. 6). 

- [12] Z. Shen, S. Liu, W. Zhu, D. Ren, Q. Xu, and Y. Feng, “A Review on Key Technologies and Developments of Hydrogen Fuel Cell Multi-Rotor Drones,” _Energies_ , vol. 17, no. 16, p. 4193, Aug. 22, 2024. Accessed: Aug. 6, 2026 (cit. on pp. 8, 9). 

- [13] F. Musolino, “Course slides of Power Electronics,” (Politecnico di Torino, Dipartimento di Elettronica e Telecomunicazioni (DET)) (cit. on pp. 13–17, 20, 31, 50, 75). 

- [14] D. Lemaitre and J. Ecrabey, “Interleaved boost converter efficiency and power density model for active and passive component design,” (cit. on p. 17). 

- [15] R. W. Erickson and D. Maksimović, _Fundamentals of Power Electronics_ , Third edition. Cham: Springer, 2020, 1 p. (cit. on p. 26). 

- [16] D.-Y. Jung, Y.-H. Ji, S.-H. Park, Y.-C. Jung, and C.-Y. Won, “Interleaved Soft-Switching Boost Converter for Photovoltaic Power-Generation System,” _IEEE Trans. Power Electron._ , vol. 26, no. 4, pp. 1137–1145, Apr. 2011. Accessed: Aug. 6, 2026 (cit. on p. 31). 

- [17] Doo-Yong Jung, Young-Hyok Ji, Jae-Hyung Kim, Chung-Yuen Won, and YongChae Jung, “Soft switching boost converter for photovoltaic power generation system,” in _2008 13th International Power Electronics and Motion Control Conference_ , Poznan, Poland: IEEE, Sep. 2008, pp. 1929–1933. Accessed: Aug. 6, 2026 (cit. on p. 31). 

- [18] L. Balogh, “Fundamentals of MOSFET and IGBT Gate Driver Circuits,” 2017 (cit. on p. 34). 

- [19] S. Kolluri and N. L. Narasamma, “Analysis, modeling, design and implementation of average current mode control for interleaved boost converter,” in _2013 IEEE 10th International Conference on Power Electronics and Drive Systems (PEDS)_ , Kitakyushu: IEEE, Apr. 2013, pp. 280–285. Accessed: Aug. 6, 2026 (cit. on pp. 50, 52). 

- [20] S. Buso and P. Mattavelli, _Digital Control in Power Electronics_ (Synthesis Lectures on Power Electronics). Cham: Springer International Publishing, 2015. Accessed: Aug. 6, 2026 (cit. on pp. 53–57). 

- [21] L. Balogh, “A Practical Introduction to Digital Power Supply Control,” (cit. on p. 57). 

- [22] “Digital Control of High-Frequency Switched-Mode Power Converters | IEEE eBooks | IEEE Xplore,” Accessed: Sep. 1, 2026 (cit. on p. 57). 

80 

_BIBLIOGRAPHY_ 

- [23] S. Mazumder, S. Tolani, and S. Joshi, “Low-Cost Single-Sensor Current Sharing Scheme for Two-Phase DC-DC Boost Converter,” in _2022 IEEE 10th Power India International Conference (PIICON)_ , New Delhi, India: IEEE, Nov. 25, 2022, pp. 1–6. Accessed: Aug. 6, 2026 (cit. on p. 64). 

- [24] H.-C. Chen, C.-Y. Lu, and L.-M. Huang, “Decoupled Current-Balancing Control With Single-Sensor Sampling-Current Strategy For Two-Phase Interleaved Boost-Type Converters,” _IEEE Trans. Ind. Electron._ , vol. 63, no. 3, pp. 1507– 1518, Mar. 2016. Accessed: Aug. 6, 2026 (cit. on p. 64). 

- [25] STMicroelectronics, _AN4539 HRTIM cookbook - Application note_ (cit. on p. 66). 

- [26] A. Fernandez, F. Tonicello, J. Aroca, and O. Mourra, “Battery discharge regulator for space applications based on the boost converter,” in _2010 TwentyFifth Annual IEEE Applied Power Electronics Conference and Exposition (APEC)_ , Feb. 2010, pp. 1792–1799. Accessed: Aug. 24, 2026 (cit. on p. 78). 

81 

# **Dedications** 

82 

