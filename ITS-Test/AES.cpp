#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>


unsigned int inverse_of[256];



class AES {
private:
	unsigned char value;
	unsigned int reduction = 283; //2^8 + 2^4 + 2^3 + 2^1 + 2^0

	int expm(AES a, AES b)
	{
		unsigned int m = b.getValue() + 283 - 1;
		AES r = 1, a_ = a.getValue(), b_ = b.getValue();
		while (m)
		{
			if (m & 1)
				r = r * a_;
			a_ = a_ * a_;
			m = m >> 1;
		}

		return r.getValue();
	}

	int euklid(int x, int b)
	{
		if (b == 0)
			return 0;
		int s, s2, t1, s1, t, t2, temp, g, r, y;
		s = s2 = t1 = 0;
		s1 = t = t2 = 1;
		y = b;
		temp = x%y;
		while (temp != 0)
		{
			g = x / y;
			r = x%y;
			s = s1 - g*s2;
			t = t1 - g*t2;
			s1 = s2;
			s2 = s;
			t1 = t2;
			t2 = t;
			x = y;
			y = r;
			temp = x%y;
		}
		if (s<0) s += b;
		return s;
	}


public:

#pragma region Konstruktoren und Getter
	unsigned int getValue()
	{
		return value;
	}

	AES()
	{
		value = 0;
	}

	AES(unsigned char x)
	{
		value = x;
	}

	AES(AES &x)
	{
		this->value = x.value;
	}
#pragma endregion

#pragma region Operatoren

	void operator=(unsigned char x)
	{
		this->value = x;
	}

	void operator=(unsigned int x)
	{
		this->value = x;
	}

	void operator=(AES x)
	{
		this->value = x.getValue();
	}

	AES operator+(AES x)
	{
		return AES(value^x.getValue());
	}

	AES operator-(AES x)
	{
		return AES(value^x.getValue());
	}
	
	AES inverse()
	{
		/* return AES(expm(getValue(), AES(reduction-2))); */
		return AES(inverse_of[getValue()]);
	}
	

	AES operator*(AES x)
	{
		unsigned int r = 0;
		unsigned int a = getValue();
		unsigned int b = x.getValue();
		while (b)
		{
			if (b & 1) 
				r = r^a;
			a = a << 1;
			if (a & (1 << 8))
				a = a^reduction;
			b = b >> 1;
		}

		return AES(r);
	}

	AES operator/(AES x)
	{
		if (x.getValue() == 0x00)
			return AES(0);
		return (*this * x.inverse());
	}
#pragma endregion

};

void initialize_AES_lookup_inverse()
{
	AES F = 0, G = 0;
	for (unsigned int i = 1; i < 256; i++)
	{
		F = i;
		for (unsigned int j = 1; j < 256; j++)
		{
			G = j;
			if ((G*F).getValue() == 1)
			{
				inverse_of[i] = j;
				inverse_of[j] = i;
			}
		}
	}
}


class State{
private:
	AES values[4][4]; //row, col

	void ShiftRow(int row, int times)
	{
		if (row == 0)
			return;
		for (int i = 0; i < times; i++)
		{
			unsigned int temp;
			temp = values[row][0].getValue();
			values[row][0] = values[row][1];
			values[row][1] = values[row][2];
			values[row][2] = values[row][3];
			values[row][3] = temp;
		}
	}

	void InvShiftRow(int row, int times)
	{
		if (row == 0)
			return;
		for (int i = 0; i < times; i++)
		{
			unsigned int temp;
			temp = values[row][3].getValue();
			values[row][3] = values[row][2];
			values[row][2] = values[row][1];
			values[row][1] = values[row][0];
			values[row][0] = temp;
		}
	}


	void MixColumn(int col)
	{
		//Save the orphans
		unsigned char temp[4][4];
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				temp[i][j] = (unsigned int)values[i][j].getValue();
			}
		}

		values[0][col] = (unsigned int)((2 * temp[0][col]) ^ (3 * temp[1][col]) ^ (temp[2][col]) ^ (temp[3][col]));
		values[1][col] = (unsigned int)((temp[0][col]) ^ (2 * temp[1][col]) ^ (3 * temp[2][col]) ^ (temp[3][col]));
		values[2][col] = (unsigned int)((temp[0][col]) ^ (temp[1][col]) ^ (2 * temp[2][col]) ^ (3 * temp[3][col]));
		values[3][col] = (unsigned int)((3 * temp[0][col]) ^ (temp[1][col]) ^ (temp[2][col]) ^ (2 * temp[3][col]));

	}

	void InvMixColumn(int col)
	{
		//Save the orphans
		unsigned char temp[4][4];
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				temp[i][j] = (unsigned int)values[i][j].getValue();
			}
		}

		values[0][col] = (unsigned int)((0x0e * temp[0][col]) ^ (0x0b * temp[1][col]) ^ (0x0d * temp[2][col]) ^ (0x09 * temp[3][col]));
		values[1][col] = (unsigned int)((0x09 * temp[0][col]) ^ (0x0e * temp[1][col]) ^ (0x0b * temp[2][col]) ^ (0x0d * temp[3][col]));
		values[2][col] = (unsigned int)((0x0d * temp[0][col]) ^ (0x09 * temp[1][col]) ^ (0x0e * temp[2][col]) ^ (0x0b * temp[3][col]));
		values[3][col] = (unsigned int)((0x0b * temp[0][col]) ^ (0x0d * temp[1][col]) ^ (0x09 * temp[2][col]) ^ (0x0e * temp[3][col]));

	}

	void SubByte(int row, int col)
	{
		unsigned int b = values[row][col].inverse().getValue();

		for (int i = 0; i < 8; i++)
		{
			b = (b) ^ (b & ((i + 4) % 8)) ^ (b&((i + 5) % 8)) ^ (b&((i + 6) % 8)) ^ (b&((i + 7) % 8)) ^ (0x63);
		}

		values[row][col] = b;
	}

	void InvSubByte(int row, int col)
	{
		unsigned int b = values[row][col].inverse().getValue();

		for (int i = 0; i < 8; i++)
		{
			b = (b) ^ (b & ((i + 4) % 8)) ^ (b&((i + 5) % 8)) ^ (b&((i + 6) % 8)) ^ (b&((i + 7) % 8)) ^ (0x63);
		}

		values[row][col] = inverse_of[b];
	}


public:

	State()
	{

	}

	State(unsigned char text[16])
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				values[i][j] = (unsigned int)text[4 * i + j];
			}
		}
	}


	unsigned char get_value(int row, int col)
	{
		return values[row][col].getValue();
	}

	void shift_rows()
	{
		for (int i = 1; i < 4; i++)
		{
			ShiftRow(i, i);
		}
	}

	void inv_shift_rows()
	{
		for (int i = 1; i < 4; i++)
		{
			InvShiftRow(i, i);
		}
	}

	void mix_columns()
	{
		for (int i = 0; i < 4; i++)
		{
			MixColumn(i);
		}
	}

	void inv_mix_columns()
	{
		for (int i = 0; i < 4; i++)
		{
			InvMixColumn(i);
		}
	}

	void sub_bytes()
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				SubByte(i, j);
			}
		}
	}

	void inv_sub_bytes()
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				InvSubByte(i, j);
			}
		}
	}

	void add_round_key(State other_state)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				values[i][j] = (unsigned char)(other_state.get_value(i, j) ^ get_value(i, j));
			}
		}
	}

	void inv_add_round_key(State other_state)
	{
		add_round_key(other_state);
	}
};

unsigned char* cipher(unsigned char in[16], unsigned char key[16])
{
	State a = State(in);
	State key_state = State(key);
	unsigned char out[16];

	a.add_round_key(key_state);

	for (int i = 1; i < 16; i++)
	{
		a.sub_bytes();
		a.shift_rows();
		a.mix_columns();
		a.add_round_key(key_state);
	}

	a.sub_bytes();
	a.shift_rows();
	a.add_round_key(key_state);

	int i, j, k=0;

	for (i = 0, k = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++, k++)
		{
			out[k] = (unsigned char)a.get_value(i, j);
		}
	}

	return out;
}

unsigned char* inv_cipher(unsigned char in[16], unsigned char key[16])
{
	State a = State(in);
	State key_state = State(key);
	unsigned char out[16];

	a.inv_add_round_key(key_state);

	for (int i = 16; i > 1; i--)
	{
		a.inv_sub_bytes();
		a.inv_shift_rows();
		a.inv_mix_columns();
		a.inv_add_round_key(key_state);
	}

	a.inv_sub_bytes();
	a.inv_shift_rows();
	a.inv_add_round_key(key_state);

	int i, j, k;

	for (i = 0, k = 0; i < 4; i++)
	{
		for (j = 0; j < 4; k++, j++)
		{
			out[k] = a.get_value(i, j);
		}
	}

	return out;
}

void main()
{
	initialize_AES_lookup_inverse();
	unsigned char *klartext = (unsigned char*)"Hallo           "; //nehm echt 16Byte.
	unsigned char *schluessel = (unsigned char*)"Schluessel      ";
	unsigned char *krypt = cipher(klartext, schluessel);
	std::cout << "Klartext: " << klartext << "\n\nVerschluesselt: " << krypt;
	unsigned char *decrpyted = inv_cipher(krypt, schluessel);

	std::cout << "\n\nEntschluesselt: " << decrpyted;
	
	system("pause");
}