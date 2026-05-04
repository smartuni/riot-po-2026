import axios from 'axios';

// Basis-URL deines Backends
const API_BASE_URL = 'http://localhost:8080';


// Erstellt eine Axios-Instanz mit Standard-Headers
const apiClient = axios.create({
    baseURL: API_BASE_URL,
    headers: {
        'Content-Type': 'application/json',
        'Accept': 'application/json'
    },
});

export default apiClient;
