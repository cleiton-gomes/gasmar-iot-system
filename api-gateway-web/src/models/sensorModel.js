
// const db = require('../config/db');

// const SensorModel = {
//     /**
//      * Salva uma nova leitura no arquivo SQLite
//      */
//     salvarLeitura(sensor_id, gas_level, alerta) {
//         return new Promise((resolve, reject) => {
//             const queryText = `
//                 INSERT INTO leituras_gas (sensor_id, gas_level, alerta) 
//                 VALUES (?, ?, ?)
//             `;
//             const values = [sensor_id, gas_level, alerta];

//             // db.run é usado para INSERT, UPDATE e DELETE
//             db.run(queryText, values, function(err) {
//                 if (err) {
//                     console.error("❌ Erro ao inserir no SQLite:", err.message);
//                     reject(err);
//                 } else {
//                     // 'this.lastID' retorna o ID da linha inserida no SQLite
//                     resolve({ id: this.lastID, sensor_id, gas_level, alerta });
//                 }
//             });
//         });
//     },

//     /**
//      * Busca as últimas 50 leituras para o histórico
//      */
//     buscarHistorico() {
//         return new Promise((resolve, reject) => {
//             const queryText = `
//                 SELECT * FROM leituras_gas 
//                 ORDER BY data_hora DESC 
//                 LIMIT 50
//             `;

//             // db.all é usado para SELECT (retorna um array de linhas)
//             db.all(queryText, [], (err, rows) => {
//                 if (err) {
//                     console.error("❌ Erro ao buscar no SQLite:", err.message);
//                     reject(err);
//                 } else {
//                     resolve(rows);
//                 }
//             });
//         });
//     }


    
    
// };

// module.exports = SensorModel;
const db = require('../config/db');

const SensorModel = {
    /**
     * Salva uma nova leitura no arquivo SQLite
     */
    salvarLeitura(sensor_id, gas_level, alerta) {
        return new Promise((resolve, reject) => {
            const queryText = `
                INSERT INTO leituras_gas (sensor_id, gas_level, alerta) 
                VALUES (?, ?, ?)
            `;
            const values = [sensor_id, gas_level, alerta];

            db.run(queryText, values, function(err) {
                if (err) {
                    console.error("❌ Erro ao inserir no SQLite:", err.message);
                    reject(err);
                } else {
                    resolve({ id: this.lastID, sensor_id, gas_level, alerta });
                }
            });
        });
    },

    /**
     * Busca as últimas 50 leituras para o histórico
     */
    buscarHistorico() {
        return new Promise((resolve, reject) => {
            const queryText = `
                SELECT * FROM leituras_gas 
                ORDER BY data_hora DESC 
                LIMIT 50
            `;
            db.all(queryText, [], (err, rows) => {
                if (err) {
                    console.error("❌ Erro ao buscar no SQLite:", err.message);
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    },

    /**
     * 🔥 Pega a última leitura salva
     */
    pegarUltimaLeitura() {
        return new Promise((resolve, reject) => {
            const queryText = `
                SELECT * FROM leituras_gas 
                ORDER BY data_hora DESC 
                LIMIT 1
            `;
            db.get(queryText, [], (err, row) => {
                if (err) {
                    console.error("❌ Erro ao buscar última leitura:", err.message);
                    reject(err);
                } else {
                    resolve(row);
                }
            });
        });
    }
};

module.exports = SensorModel;