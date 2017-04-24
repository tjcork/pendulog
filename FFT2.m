xTM=flip(TM);
xPT=flip(PT);

xTstart=xStart.Position(1)
xTend=xEnd.Position(1)

xTM=xTM(find(xTM==xTstart):find(xTM==xTend));
xPT=detrend(xPT(find(xTM==xTstart):find(xTM==xTend)));

[zPT,zTM] = resample(xPT,xTM,0.02,'spline');



data=zPT;
n = length(data);
NFFT = 2^nextpow2(n); % Next power of 2 from length of data
Y = fft(data,NFFT)/n;
f = Fs/2*linspace(0,1,NFFT/2+1);
freqamp=Y(1:NFFT/2+1);
[d,x]=max(abs(freqamp(100:end)));
f(x+99)
figure(1) 
plot(f,2*abs(freqamp))


hold on


interval=600000;
for i=0:interval:1000000-interval
    pTM=xTM(i+1:i+interval);
    pPT=xPT(i+1:i+interval);
    %[data,Ty] = resample(pPT,pTM,0.02,'spline');
    data=pPT;
    n = length(data);
    NFFT = 2^nextpow2(n); % Next power of 2 from length of data
    Y = fft(data,NFFT)/n;
    f = Fs/2*linspace(0,1,NFFT/2+1);
    freqamp=Y(1:NFFT/2+1);
    [d,x]=max(abs(freqamp(100:end)));
    f(x+99)
    hold on 
    plot(f+0.0015,2*abs(freqamp).*(1+(randn(length(freqamp),1)/10000)))
end


