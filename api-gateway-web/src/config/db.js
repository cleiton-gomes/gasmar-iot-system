// const { Pool } = require('pg');

// const pool = new Pool({
//     user: 'seu_usuario',
//     host: 'localhost',
//     database: 'nome_do_seu_banco',
//     password: 'sua_senha',
//     port: 5432,
// });

// // module.exports = pool;
// require('dotenv').config(); // Carrega as variáveis do .env
// const { Pool } = require('pg');

// const pool = new Pool({
//     user: process.env.DB_USER,
//     host: process.env.DB_HOST,
//     database: process.env.DB_NAME,
//     password: process.env.DB_PASSWORD,
//     port: process.env.DB_PORT,
// });
// // Função que simula uma "Migrate"
// const inicializarBanco = async () => {
//     const queryText = `
//         CREATE TABLE IF NOT EXISTS leituras_gas (
//             id SERIAL PRIMARY KEY,
//             sensor_id VARCHAR(50) NOT NULL,
//             gas_level INTEGER NOT NULL,
//             alerta BOOLEAN DEFAULT FALSE,
//             data_hora TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
//         );
//     `;
//     try {
//         await pool.query(queryText);
//         console.log("✅ Tabelas verificadas/criadas com sucesso (Migrate OK).");
//     } catch (err) {
//         console.error("❌ Erro ao rodar migrate manual:", err);
//     }
// };

// // Executa a inicialização
// inicializarBanco();

// // --- BLOCO DE TESTE ---
// pool.query('SELECT NOW()', (err, res) => {
//     if (err) {
//         console.error('❌ ERRO DE CONEXÃO NO POSTGRES:');
//         console.error('Motivo:', err.message);
//         console.log('Verifique se o pgAdmin está aberto e se a senha no .env está correta.');
//     } else {
//         console.log('✅ CONECTADO AO POSTGRES COM SUCESSO!');
//         console.log('Hora no banco:', res.rows[0].now);
//     }
// });

// module.exports = pool;const sqlite3 = require('sqlite3').verbose();
const sqlite3 = require('sqlite3').verbose();
const path = require('path');

// Define o caminho do arquivo do banco na raiz do projeto
const dbPath = path.resolve(__dirname, '../../database.sqlite');

// Conecta ao SQLite (se o arquivo não existir, ele será criado automaticamente)
const db = new sqlite3.Database(dbPath, (err) => {
    if (err) {
        console.error('❌ ERRO AO CONECTAR AO SQLITE:', err.message);
    } else {
        console.log('✅ CONECTADO AO SQLITE COM SUCESSO!');
        console.log('Arquivo do banco:', dbPath);
        inicializarBanco();
    }
});

// Função de Inicialização (Equivalente à Migrate)
const inicializarBanco = () => {
    const queryText = `
        CREATE TABLE IF NOT EXISTS leituras_gas (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            sensor_id TEXT NOT NULL,
            gas_level INTEGER NOT NULL,
            alerta BOOLEAN DEFAULT 0,
            data_hora DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    `;

    db.run(queryText, (err) => {
        if (err) {
            console.error("❌ Erro ao criar tabela no SQLite:", err.message);
        } else {
            console.log("✅ Tabela 'leituras_gas' verificada/criada (SQLite OK).");
        }
    });
};

module.exports = db;