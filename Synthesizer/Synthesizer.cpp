
#include <iostream>
#include "NoiseMaker.h"

atomic<double> m_Frequency = 0.0; // Dominant output frequency
double octaveFrequency = 110.0; // Frequency of octave 
double twThRootOf2 = pow(2.0, 1.0 / 12.0); //12 notes per Octave 

struct EnvelopeADSR //stuct for Attack/Decay/Sustain and Release 
{
	double m_AttackTime;
	double m_DecayTime;
	double m_ReleaseTime;

	double m_Sustain;
	double m_StartAplitude;

	double m_PressedTime;
	double m_ReleasedTime;

	bool isPressed;

	EnvelopeADSR()
	{
		m_AttackTime = 0.100; // 100 milliseconds
		m_DecayTime = 0.01;
		m_StartAplitude = 1.0;
		m_Sustain = 0.8;
		m_ReleaseTime = 0.200; // 200 milliseconds
		m_PressedTime = 0.0;
		m_ReleasedTime = 0.0;

		isPressed = false;
	}

	double GetAmplitude(double deltaT)
	{
		double amplitude = 0.0;
		double envelopeLifeT = deltaT - m_PressedTime;

		if (isPressed)
		{
			//Ads

			//Attack phase
			if (envelopeLifeT <= m_AttackTime)
			{
				amplitude = (envelopeLifeT / m_AttackTime) * m_StartAplitude; //Normalizing attack time/
				//Creating a value between 0 and 1, if the lifetime is less than attack time, going to no amplitude to 100% amplitude
				//
			}

			//Decay phase
			if (envelopeLifeT > m_AttackTime && envelopeLifeT <= (m_AttackTime + m_DecayTime))
			{
				amplitude = ((envelopeLifeT - m_AttackTime) / m_DecayTime) *
					(m_Sustain - m_StartAplitude) + m_StartAplitude;
				//Creating a value between 0 and 1 on how far we are in decay time
			}

			//Sustain phase
			if (envelopeLifeT > (m_AttackTime + m_DecayTime))
			{
				amplitude = m_Sustain;
			}

		}
		else
		{
			//Release phase
			amplitude = ((deltaT - m_ReleasedTime) / m_ReleasedTime) * (0.0 - m_Sustain) + m_Sustain;
			//Creating a value between 0 an 1 on how far wer are in release phase
		}

		//Stopping low frequency sound that cant be heard 
		if (amplitude <= 0.0001)
		{
			amplitude = 0;
		}



		return amplitude;
	}


	void NotePressed(double time)
	{
		m_PressedTime = time;
		isPressed = true;
	}

	void NoteReleased(double time)
	{
		m_ReleasedTime = time;
		isPressed = false;
	}

};

EnvelopeADSR* m_Envelope;

//Converting frequency to angular velocity
double ConvertF(double hertz)
{
	return hertz * 2.0 * PI;
}

double oscillator(double hertz, double deltaT, int type)
{
	double output = 0.0;
	switch (type)
	{
	case 0:
		return sin(ConvertF(hertz) * deltaT); //sine wave

	case 1:
		return sin(ConvertF(hertz) * deltaT) > 0.0 ? 1.0 : -1.0; //Square wave

	case 2:
		return asin(sin(ConvertF(hertz) * deltaT)) * 2.0 / PI; //Triangle wave

	case 3:

		for (double i = 1.0; i < 10.0; i++)
		{
			output += (sin(i * ConvertF(hertz) * deltaT) / i);
		}

		return output * (2.0 / PI); // Saw wave (analoque, warm, slow)

	case 4:

		return (2.0 / PI) * (hertz * PI * fmod(deltaT, 1.0 / hertz) - (PI / 2.0));
		//Saw wave(optimized, harsh, fast)

	case 5:

		return 2.0 * ((double)rand() / (double)RAND_MAX) - 1.0;


	default:
		return 0.0;
		break;
	}
}


double Noise(double deltaT)
{
	//Making a square wave
	//double outout = 1.0 * sin(m_Frequency * 2 * 3.14159 * deltaT);

	//Making a chord
	//double outout = 1.0 * (sin(ConvertF(m_Frequency) * deltaT));

	//Calling oscillator
	//double outout = oscillator(m_Frequency, deltaT, 5);
		
	//Envelope call
	//Instead of hard coding using envelope to get the right time
	double outout = m_Envelope->GetAmplitude(deltaT) * oscillator(m_Frequency, deltaT, 3);

	return outout * 0.4; // master volume

	/*if (outout > 0.0)
	{
		return 0.3;
	}
	else
	{
		return -0.3;
	}*/

	//2 * PI to return angular velocity so Sin can understand it
	//return 0.5 * sin(220.0 * 2 * 3.14159 * deltaT);
}




int main()
{
	m_Envelope = new EnvelopeADSR;
	//Get all sound hardware
	vector<wstring> devices = olcNoiseMaker<short>::Enumerate();

	//Display findinds
	for (auto d : devices) wcout << "Found Output Device: " << d << endl;


	//Create sound machine

	//Char it becames a 8 bits system
	//Int it becomes a 32 bits system
	//Using short because is 16 bits, so short is used to store the accuracy of the amplitude
	//44100 is the sample rate is  used to store accuracy of the frequency 
	olcNoiseMaker<short> sound(devices[0] , 44100, 1 , 8, 512);

	//Link noise function with sound machine
	sound.SetUserFunction(Noise);


	int currentKey = -1;
	bool keyPressed = false;
	
	while (1)
	{

		//Keyboard
		bool keyPressed = false;
		for (int i = 0; i < 15; i++)
		{
			//Making the keyboard like a piano White and black keys
			if (GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbc"[i])) & 0x8000)
			{
				if (currentKey != i)
				{
					m_Frequency = octaveFrequency * pow(twThRootOf2, i);
					m_Envelope->NotePressed(sound.GetTime());
					currentKey = i;
				}
				keyPressed = true;
			}
		}

		if (!keyPressed)
		{
			if (currentKey != -1)
			{
				m_Envelope->NoteReleased(sound.GetTime());

				currentKey = -1;
			}
		}

		//One key
		//if (GetAsyncKeyState('A') & 0x8000)
		//{
		//	m_Frequency = octaveFrequency * pow(twThRootOf2, 1);
		//}
		//else
		//{
		//	m_Frequency = 0.0;
		//}
	}


	return 0;
}
