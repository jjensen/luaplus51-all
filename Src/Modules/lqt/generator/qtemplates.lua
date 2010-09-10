return {
	-- example usage:
	--
	-- ["ex<T>"] = { "ex<double>", "ex<string>" }
	-- module = { ["ex2<K,V>"] = { "example<char*, int>" } }
	
	qtcore = {
		["QList<T>"] = { "QList<QString>", "QList<QFileInfo>" },
	},
	qtgui = {
		["QList<T>"] = { "QList<QGraphicsItem*>" },
		["QVector<T>"] = { "QVector<QPointF>" },
	},
}
