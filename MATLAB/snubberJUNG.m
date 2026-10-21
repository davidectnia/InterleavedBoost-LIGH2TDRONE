% =========================================================================
% Progettazione Rete Snubber Risonante (ISSBC) - Con Verifica Valori Commerciali
% =========================================================================
clear all
close all
clc
format shortEng
% --- 1. Specifiche di Input del Convertitore ---
Vin_max = 18;       % [V] Tensione massima in ingresso
Vo = 44.4;          % [V] Tensione di uscita
Po = 100;           % [W] Potenza di uscita
fsw = 400e3;        % [Hz] Frequenza di commutazione (400 kHz)
L = 27e-6;          % [H] Induttanza principale
Coes = 1000e-12;    % [F] Capacità di uscita del transistor (es. MOSFET)

% --- 2. Calcoli Preliminari (Caso peggiore a Vin_max) ---
Dmin = 1 - (Vin_max / Vo);            % Duty cycle minimo
Iin = Po / Vin_max;                   % Corrente media di ingresso
delta_IL = (Vin_max * Dmin)/(L * fsw); % Ripple di corrente
Imin = Iin - (delta_IL / 2);          % Corrente minima dell'induttore prima della commutazione

fprintf('--- SPECIFICHE E PARAMETRI BASE ---\n');
fprintf('Dmin = %.3f\n', Dmin);
fprintf('Imin = %.2f A\n\n', Imin);

% --- 3. Progettazione della Capacità Ausiliaria Ca (Eq. 34) ---
Ca_min = 20 * Coes;
Ca = 22e-9; % Valore commerciale standard scelto [22 nF]
fprintf('--- 1. SCELTA CAPACITA'' AUSILIARIA (Ca) ---\n');
fprintf('Ca minimo richiesto = %.2f nF -> Scelto Ca = %.1f nF\n\n', Ca_min*1e9, Ca*1e9);

% --- 4. Disaccoppiamento: Scelta della Corrente di Picco Teorica I2 ---
% Target iniziale: I2 circa il doppio di Imin
I2_target = 2 * Imin;

% --- 5. Progettazione di Cr (Eq. 39 Corretta) ---
term1_corr = (Dmin * (I2_target - Imin)) / (pi * Vo * fsw);
term2_corr = sqrt(1 - (Imin^2) / ((pi^2) * (I2_target - Imin)^2));
Cr_min_vero = term1_corr / term2_corr;

Cr = 68e-9; % Valore commerciale scelto [68 nF]
fprintf('--- 2. SCELTA CAPACITA'' RISONANTE (Cr) ---\n');
fprintf('Cr minimo teorico = %.2f nF -> Scelto Cr commerciale = %.1f nF\n\n', Cr_min_vero*1e9, Cr*1e9);

% --- 6. Calcolo di Lr Teorico in base a Cr scelto (Eq. 37) ---
Lr_teorico = (Vo / (I2_target - Imin))^2 * Cr;
fprintf('--- 3. SCELTA INDUTTORE RISONANTE (Lr) ---\n');
fprintf('Lr teorico calcolato con Eq(37) = %.2f uH\n', Lr_teorico*1e6);

Lr = 4.7e-6; % Valore commerciale standard scelto (es. 4.7 uH)
fprintf('-> Scelto Lr commerciale = %.1f uH\n\n', Lr*1e6);

% --- 7. VERIFICA FINALE SUI VALORI COMMERCIALI SCELTI ---
fprintf('--- 4. VERIFICA DESIGN CON COMPONENTI COMMERCIALI ---\n');

% A. Verifica della VERA corrente di picco (I2)
% Invertiamo l'Eq(37) usando Lr e Cr commerciali: I2_real = Imin + Vo * sqrt(Cr / Lr)
I2_real = Imin + Vo * sqrt(Cr / Lr);
fprintf('Corrente di picco REALE I2 = %.2f A (Target iniziale era %.2f A)\n', I2_real, I2_target);

if I2_real > (1.2 * Imin)
    fprintf('  [OK] La corrente reale I2 assicura energia sufficiente per lo ZVS.\n');
elseif I2_real > Imin
    fprintf('  [WARNING] La corrente I2 e'' poco piu'' grande di Imin. Lo ZVS potrebbe essere parziale a pieno carico.\n');
else
    fprintf('  [ERRORE] La corrente I2 e'' inferiore a Imin. ZVS impossibile con questi componenti!\n');
end

% B. Verifica del limite massimo di Lr (Eq. 40)
Cs = (Cr * Ca) / (Cr + Ca);
% Termine che determina se la formula è applicabile
term_num = (2 * 0.85 * (Cs / Ca) - 1);

if term_num <= 0
    fprintf('\n  [INFO] Il termine matematico della Eq 40 e'' <= 0.\n');
    fprintf('  Questo significa che il modello analitico semplificato del paper fallisce a\n');
    fprintf('  400kHz con i rapporti capacitivi correnti. Ignorare la Eq 40 e validare tramite simulazione SPICE.\n');
else
    % Usiamo la corrente reale I2_real per la verifica!
    numeratore = term_num * Vo;
    denominatore = (I2_real / (Cr + Ca)) * pi * sqrt(Cs);
    Lr_max_eq40 = (numeratore / denominatore)^2;
    
    fprintf('\nLimite massimo teorico Lr (Eq 40) = %.2f uH\n', Lr_max_eq40*1e6);
    if Lr < Lr_max_eq40
        fprintf('  [OK] Lr scelto (%.1f uH) rispetta la disuguaglianza (minore di %.1f uH).\n', Lr*1e6, Lr_max_eq40*1e6);
    else
        fprintf('  [WARNING] Lr scelto supera il limite. Verificare i tempi di salita/discesa in simulazione.\n');
    end
end
fprintf('\n=========================================================================\n');