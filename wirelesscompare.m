%data plot
datafile='data-LadderPendulum.csv';

%[D, T, R] = xlsread(datafile);
%wTM = flip(D(:,2));                 % Data Channel
%wPT = flip(D(:,4));

Ts=50E-3;
Fs = 1/Ts; % sampling frequency

%test 1
%1492351344435
%1492352433240
t1start=1492351344435;
t1end=1492352433240;

%t1TM=wTM(find(wTM==t1start):find(wTM==t1end));
%t1PT=wPT(find(wTM==t1start):find(wTM==t1end));

t1TM=wTM(find(wTM < t1end-1000000, 1, 'last'):find(wTM==t1end));
t1PT=detrend(wPT(find(wTM < t1end-1000000, 1, 'last'):find(wTM==t1end)));
[t1PT,t1TM] = resample(t1PT,t1TM,0.02,'spline');

%test Wireless power
%1492357783013
%1492358902596

t2start=1492357783013;
t2end=1492358902596;

%t2TM=wTM(find(wTM==t2start):find(wTM==t2end));
%t2PT=wPT(find(wTM==t2start):find(wTM==t2end));

t2TM=wTM(find(wTM < t2end-1000000, 1, 'last'):find(wTM==t2end));
t2PT=detrend(wPT(find(wTM < t2end-1000000, 1, 'last'):find(wTM==t2end)));
[t2PT,t2TM] = resample(t2PT,t2TM,0.02,'spline');

%-------------FFT----------------------

data=t2PT;
n = length(data);
NFFT = 2^nextpow2(n); % Next power of 2 from length of data
Y = fft(data,NFFT)/n;
f = Fs/2*linspace(0,1,NFFT/2+1);
freqamp=Y(1:NFFT/2+1);
[d,x]=max(abs(freqamp(100:end)));
f(x+99)
figure(1)
hold on
plot(f,2*abs(freqamp))


data=t1PT;
n = length(data);
NFFT = 2^nextpow2(n); % Next power of 2 from length of data
Y = fft(data,NFFT)/n;
f = Fs/2*linspace(0,1,NFFT/2+1);
freqamp=Y(1:NFFT/2+1);
[d,x]=max(abs(freqamp(100:end)));
f(x+99)
figure(1) 
plot(f,2*abs(freqamp))



%-----------------Q-----------------------

[pks1,locs1]=findpeaks(t1PT,t1TM);
[pks2,locs2]=findpeaks(t2PT,t2TM);

t1PTs=t1PT(find(t1TM==locs1(find(pks1 <= 4, 1, 'first'))):end);
t1TMs=t1TM(find(t1TM==locs1(find(pks1 <= 4, 1, 'first'))):end);

Q1=(find(pks1 <= (t1PTs(1) / exp(1)),1,'first') - find(pks1 <= 4, 1, 'first'))*2*pi()

t2PTs=t2PT(find(t2TM==locs2(find(pks2 <= 4, 1, 'first'))):length(t1PTs));
t2TMs=t2TM(find(t2TM==locs2(find(pks2 <= 4, 1, 'first'))):length(t1PTs));

Q2=(find(pks2 <= (t2PTs(1) / exp(1)),1,'first') - find(pks2 <= 4, 1, 'first'))*2*pi()

%--------------PLOT-----------------------

figure(2);
plot(t2TMs-t2TMs(1),t2PTs)
hold on
plot(t1TMs-t1TMs(1),t1PTs)


ylabel('Amplitude (degrees)')
xlabel('Time (minutes)')
xticks(60*1000*[0 2 4 6 8 10 12 14 16 18 20 22])
xticklabels({'0','2','4','6','8','10','12','14','16','18','20','22'})

%-------------FFT2------------------------


data=t2PTs;
n = length(data);
NFFT = 2^nextpow2(n); % Next power of 2 from length of data
Y = fft(data,NFFT)/n;
f = Fs/2*linspace(0,1,NFFT/2+1);
freqamp=Y(1:NFFT/2+1);
[d,x]=max(abs(freqamp(100:end)));
f(x+99)
figure(1)
hold on
plot(f,2*abs(freqamp))



data=t1PTs;
n = length(data);
NFFT = 2^nextpow2(n); % Next power of 2 from length of data
Y = fft(data,NFFT)/n;
f = Fs/2*linspace(0,1,NFFT/2+1);
freqamp=Y(1:NFFT/2+1);
[d,x]=max(abs(freqamp(100:end)));
f(x+99)
figure(1)
hold on
plot(f,2*abs(freqamp))







