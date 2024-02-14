const temperatureCtx = document.getElementById('temperatureChart').getContext('2d');
const humidityCtx = document.getElementById('humidityChart').getContext('2d');

// APIからデータを取得する関数
async function fetchData() {
    const response = await fetch('/data');
    const data = await response.json();
    return data;
}

function updateLatestMeasurements(data) {
    // 直近のデータを取得
    const latestData = data[0];
    document.getElementById('latestTemperature').textContent = latestData.temperature.toFixed(1);
    document.getElementById('latestHumidity').textContent = latestData.humidity.toFixed(1);
}

function updateDataUpdateTime(data) {
    if (data && data.length > 0) {
        // 最新のデータのタイムスタンプを取得
        const latestTime = new Date(data[0].timestamp);
        const month = (latestTime.getMonth() + 1); // 月（0から始まるため、1を加える）
        const day = latestTime.getDate(); // 日
        const hours = latestTime.getHours().toString().padStart(2, '0'); // 時
        const minutes = latestTime.getMinutes().toString().padStart(2, '0'); // 分
        const seconds = latestTime.getSeconds().toString().padStart(2, '0'); // 秒
        // タイムスタンプを任意のフォーマットに変換（例: "YYYY-MM-DD HH:mm:ss"）
        const formattedTimestamp = `${month}月${day}日 ${hours}:${minutes}:${seconds}`;
        // 更新時刻を表示
        document.getElementById('dataUpdateTime').textContent = `データ更新時刻: ${formattedTimestamp}`;
    }
}

// 時間単位でデータをグループ化し、各グループの平均値を計算する関数
function processAverageData(data) {
    const groupedData = {};
    const now = new Date(data[0].timestamp);
    const oneDayAgo = new Date(now.getTime() - 23*60*60*1000);

    // 過去23時間の各時間帯に対してデータを初期化
    for(let hour = oneDayAgo; hour <= now; hour.setHours(hour.getHours() + 1)) {
        const key = hour.toISOString().substring(0, 13); // "YYYY-MM-DDTHH"形式のキー
        groupedData[key] = { temperature: [], humidity: [] };
    }

    // データを時間帯ごとにグループ化
    data.forEach(entry => {
        const date = new Date(entry.timestamp);
        const key = date.toISOString().substring(0, 13); // "YYYY-MM-DDTHH"形式のキー
        if(groupedData.hasOwnProperty(key)) {
            groupedData[key].temperature.push(entry.temperature);
            groupedData[key].humidity.push(entry.humidity);
        }
    });

    const labels = [];
    const tempAverages = [];
    const humidityAverages = [];

    Object.keys(groupedData).forEach(key => {
        // ISO 8601形式に準じた日付文字列を作成（時刻を含む）
        const date = new Date(`${key}:00:00Z`); // "Z"を追加してUTCとして解釈させる
        const offset = 9; // GMT+9のオフセット
        const localDate = new Date(date.getTime() + offset * 60 * 60 * 1000); // UTC時間に9時間を加算
        const hours = localDate.getUTCHours().toString().padStart(2, '0'); // 変換後の時間を取得
        labels.push(hours); // "HH"形式のラベルとして追加

        // 温度の平均値を計算
        const temps = groupedData[key].temperature;
        const tempAverage = temps.length ? temps.reduce((a, b) => a + b, 0) / temps.length : 0;
        tempAverages.push(tempAverage);

        // 湿度の平均値を計算
        const humids = groupedData[key].humidity;
        const humidityAverage = humids.length ? humids.reduce((a, b) => a + b, 0) / humids.length : 0;
        humidityAverages.push(humidityAverage);
    });

    return { labels, tempAverages, humidityAverages };
}

// グラフを描画する関数
async function drawChart() {
    const rawData = await fetchData();
    updateLatestMeasurements(rawData); // 直近の測定値を更新
    updateDataUpdateTime(rawData); // データ更新時刻を更新
    const { labels, tempAverages, humidityAverages } = processAverageData(rawData);

    // 温度グラフ
    new Chart(temperatureCtx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: [{
                label: '',
                fill: 'origin',
                backgroundColor: 'rgba(255, 99, 132, 0.2)',
                borderColor: 'rgba(255, 99, 132, 1)',
                data: tempAverages,

            }]
        },
        options: {
            aspectRatio: 1.8,
            plugins: {
                legend: {
                    display: false
                }
            },
            layout: {
                padding: 10
            },
            scales: {
                y: {
                    suggestedMin: 10,
                    suggestedMax: 30
                }                 
            }
        }
    });

    // 湿度グラフ
    new Chart(humidityCtx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: [{
                label: '',
                fill: 'origin',
                backgroundColor: 'rgba(54, 162, 235, 0.2)',
                borderColor: 'rgba(54, 162, 235, 1)',
                data: humidityAverages,
            }]
        },
        options: {
            aspectRatio: 1.8,
            plugins: {
                legend: {
                    display: false
                }
            },  
            layout: {
                padding: 10
            }, 
            scales: {
                y: {
                    suggestedMin: 25,
                    suggestedMax: 75
                }                 
            }
        }
    });
}

drawChart();

