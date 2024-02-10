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
    const latestData = data[data.length - 1];
    document.getElementById('latestTemperature').textContent = latestData.temperature.toFixed(1);
    document.getElementById('latestHumidity').textContent = latestData.humidity.toFixed(1);
}

// 時間単位でデータをグループ化し、各グループの平均値を計算する関数
function processAverageData(data) {
    const groupedData = {};
    const now = new Date();
    const oneDayAgo = new Date(now.getTime() - 24*60*60*1000);

    // 過去24時間の各時間帯に対してデータを初期化
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
        labels.push(key + ":00"); // ラベルに":00"を追加して、時刻を示す

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
    const { labels, tempAverages, humidityAverages } = processAverageData(rawData);

    // 温度グラフ
    new Chart(temperatureCtx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: [{
                label: 'Average Temperature (°C)',
                backgroundColor: 'rgba(255, 99, 132, 0.2)',
                borderColor: 'rgba(255, 99, 132, 1)',
                data: tempAverages,
            }]
        },
        options: {
            scales: {
                y: {
                    beginAtZero: true
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
                label: 'Average Humidity (%)',
                backgroundColor: 'rgba(54, 162, 235, 0.2)',
                borderColor: 'rgba(54, 162, 235, 1)',
                data: humidityAverages,
            }]
        },
        options: {
            scales: {
                y: {
                    beginAtZero: true
                }
            }
        }
    });
}

drawChart();

