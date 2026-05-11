import js from '@eslint/js';
import globals from 'globals';
import react from 'eslint-plugin-react';
import importX from 'eslint-plugin-import-x';

export default [
    js.configs.recommended,
    react.configs.flat['jsx-runtime'],
    importX.configs['flat/recommended'],
    {
        files: ['src/**/*.{js,jsx}'],
        plugins: {
            'react': react,
        },
        rules: {
            'react/jsx-uses-vars': 'error',
            'react/react-in-jsx-scope': 'off',
            'react/prop-types': 'off',
            'import-x/no-duplicates': 'error',
        },
        languageOptions: {
            globals: {
                ...globals.browser,
                ...globals.es2020,
            },
        },
        settings: {
            react: {
                version: 'detect',
            },
            'import-x/resolver': {
                node: {
                    extensions: ['.js', '.jsx'],
                },
            },
        },
    },
];