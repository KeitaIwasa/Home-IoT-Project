const express = require('express');
const {Datastore} = require('@google-cloud/datastore');
const app = express();
const port = 8080;

// Google Cloud Datastoreのインスタンスを初期化
const datastore = new Datastore();

// 静的ファイルを提供するディレクトリを設定
app.use(express.static('public'));

app.use(express.json());

app.post('/data', async (req, res) => {
    const { temperature, humidity } = req.body; // リクエストボディからデータを抽出
    console.log(`Received temperature: ${temperature}, humidity: ${humidity}`);

    const kind = 'SensorData'; // Datastoreのエンティティタイプ
    const dataKey = datastore.key([kind]);
    const entity = {
        key: dataKey,
        data: [
            { name: 'temperature', value: temperature },
            { name: 'humidity', value: humidity },
            { name: 'timestamp', value: new Date() } // タイムスタンプを追加
        ],
    };

    try {
        await datastore.save(entity);
        res.status(200).send('Data received and stored.');
    } catch (error) {
        console.error('Error saving data:', error);
        res.status(500).send('Failed to save data.');
    }
});


app.get('/data', async (req, res) => {
    // 現在時刻から23時間前の時刻を計算
    const oneDayAgo = new Date();
    oneDayAgo.setHours(oneDayAgo.getHours() - 23);

    const query = datastore.createQuery('SensorData')
        .filter('timestamp', '>', oneDayAgo)
        .order('timestamp', {descending: true});

    try {
        const [entities] = await datastore.runQuery(query);
        const data = entities.map(entity => {
            const dataObject = {
                id: entity[datastore.KEY].id,
                temperature: entity.temperature,
                humidity: entity.humidity,
                timestamp: entity.timestamp
            };
            return dataObject;
        });
        res.json(data);
    } catch (error) {
        console.error('Error fetching data:', error);
        res.status(500).send('Failed to fetch data');
    }
});
  

// '/dashboard'エンドポイントへのGETリクエストをdashboard.htmlにリダイレクト
app.get('/dashboard', (req, res) => {
    res.sendFile(__dirname + '/public/dashboard.html');
});



app.listen(port, () => {
    console.log(`Server listening at http://localhost:${port}`);
});
