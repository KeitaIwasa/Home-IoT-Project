const ctx = document.getElementById('tempHumidityChart').getContext('2d');

// APIからデータを取得する関数
async function fetchData() {
    const response = await fetch('/data');
    const data = await response.json();
    return data;
}

// 時間単位でデータをグループ化し、各グループの平均値を計算する関数
function processAverageData(data) {
    // グループ化されたデータを保持するオブジェクト
    const groupedData = {};

    // データをグループ化
    data.forEach(entry => {
        const date = new Date(entry.timestamp);
        // 時間単位でキーを生成（例: '2024-02-09T15'）
        const key = `${date.getFullYear()}-${date.getMonth() + 1}-${date.getDate()}T${date.getHours()}`;
        if (!groupedData[key]) {
            groupedData[key] = { temperature: [], humidity: [] };
        }
        groupedData[key].temperature.push(entry.temperature);
        groupedData[key].humidity.push(entry.humidity);
    });

    // 各グループの平均値を計算
    const labels = [];
    const tempAverages = [];
    const humidityAverages = [];
    Object.keys(groupedData).forEach(key => {
        labels.push(key);
        const tempAvg = groupedData[key].temperature.reduce((a, b) => a + b, 0) / groupedData[key].temperature.length;
        const humidityAvg = groupedData[key].humidity.reduce((a, b) => a + b, 0) / groupedData[key].humidity.length;
        tempAverages.push(tempAvg);
        humidityAverages.push(humidityAvg);
    });

    return { labels, tempAverages, humidityAverages };
}

// グラフを描画する関数
async function drawChart() {
    const rawData = await fetchData();
    const { labels, tempAverages, humidityAverages } = processAverageData(rawData);

    const chart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: [{
                label: 'Average Temperature (°C)',
                backgroundColor: 'rgba(255, 99, 132, 0.2)',
                borderColor: 'rgba(255, 99, 132, 1)',
                yAxisID: 'y',
                data: tempAverages,
            }, {
                label: 'Average Humidity (%)',
                backgroundColor: 'rgba(54, 162, 235, 0.2)',
                borderColor: 'rgba(54, 162, 235, 1)',
                yAxisID: 'y1',
                data: humidityAverages,
            }]
        },
        options: {
            scales: {
                y: {
                    type: 'linear',
                    display: true,
                    position: 'left',
                },
                y1: {
                    type: 'linear',
                    display: true,
                    position: 'right',
                    grid: {
                        drawOnChartArea: false,
                    },
                }
            }
        }
    });
}

drawChart();

