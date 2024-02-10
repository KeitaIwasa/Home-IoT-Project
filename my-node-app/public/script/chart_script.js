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

    // 期間内のすべての時間帯にデータを初期化（ここで欠損データを0で扱う）
    const startHour = new Date(data[0].timestamp);
    startHour.setMinutes(0, 0, 0); // 最初の時間帯を設定
    const endHour = new Date();
    endHour.setMinutes(59, 59, 999); // 現在の時間帯を設定

    for (let hour = startHour; hour <= endHour; hour.setHours(hour.getHours() + 1)) {
        const key = `${hour.getFullYear()}-${hour.getMonth() + 1}-${hour.getDate()}T${hour.getHours()}`;
        groupedData[key] = { temperature: [0], humidity: [0] }; // 欠損データを0で初期化
    }

    // データをグループ化し、実際にデータが存在する場合は0を上書き
    data.forEach(entry => {
        const date = new Date(entry.timestamp);
        const key = `${date.getFullYear()}-${date.getMonth() + 1}-${date.getDate()}T${date.getHours()}`;
        if (groupedData[key]) { // すでにキーが存在する場合、0を上書き
            groupedData[key].temperature[0] = entry.temperature;
            groupedData[key].humidity[0] = entry.humidity;
        } else { // キーが存在しない場合（理論上はすべての時間帯が初期化されているため発生しないはず）
            groupedData[key] = {
                temperature: [entry.temperature],
                humidity: [entry.humidity]
            };
        }
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

