class DialoguePlayerState
{
	ref array<string> Names;
	ref array<int> Values;

	void DialoguePlayerState()
	{
		Names = new array<string>;
		Values = new array<int>;
	}

	void Sanitize()
	{
		if (!Names)
			Names = new array<string>;
		if (!Values)
			Values = new array<int>;

		while (Values.Count() < Names.Count())
			Values.Insert(0);
	}

	int Get(string name)
	{
		int idx = Names.Find(name);
		if (idx < 0 || idx >= Values.Count())
			return 0;
		return Values[idx];
	}

	void Set(string name, int value)
	{
		int idx = Names.Find(name);
		if (idx < 0)
		{
			Names.Insert(name);
			Values.Insert(value);
		}
		else
		{
			Values[idx] = value;
		}
	}

	void OnSend(ScriptRPC rpc)
	{
		rpc.Write(Names.Count());
		foreach (string name : Names)
			rpc.Write(name);

		rpc.Write(Values.Count());
		foreach (int value : Values)
			rpc.Write(value);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		int nameCount;
		if (!ctx.Read(nameCount)) return false;
		Names.Clear();
		for (int i = 0; i < nameCount; i++)
		{
			string name;
			if (!ctx.Read(name)) return false;
			Names.Insert(name);
		}

		int valueCount;
		if (!ctx.Read(valueCount)) return false;
		Values.Clear();
		for (int j = 0; j < valueCount; j++)
		{
			int value;
			if (!ctx.Read(value)) return false;
			Values.Insert(value);
		}

		return true;
	}
}
