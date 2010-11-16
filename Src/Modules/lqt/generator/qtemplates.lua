return {
	-- example usage:
	--
	-- ["ex<T>"] = { "ex<double>", "ex<string>" }
	-- module = { ["ex2<K,V>"] = { "example<char*, int>" } }
	
	qtcore = {
		["QList<T>"] = { "QList<QString>", "QList<QFileInfo>", "QList<QVariant>" },
	},
	qtgui = {
		["QList<T>"] = { "QList<QGraphicsItem*>", "QList<int>", "QList<qreal>",
			"QList<QModelIndex>", "QList<QSize>", "QList<QPolygonF>", "QList<QKeySequence>",
			"QList<QUrl>" },
		["QVector<T>"] = { "QVector<QPointF>", "QVector<QPoint>", "QVector<QRgb>", "QVector<QLine>",
			"QList<QRectF>", "QVector<QTextLength>", "QVector<QGradientStop>" },
	},
	qtnetwork = {
		["QList<T>"] = { "QList<QSslError>", "QList<QSslCertificate>", "QList<QNetworkCookie>",
			"QList<QSslCipher>", "QList<QNetworkAddressEntry>", "QList<QNetworkProxy>", 
			"QList<QHostAddress>", }
	},
}
