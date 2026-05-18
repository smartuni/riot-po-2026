import js from '@eslint/js';
import globals from 'globals';
import react from 'eslint-plugin-react';
import importX from 'eslint-plugin-import-x';
import reactHooks from 'eslint-plugin-react-hooks';
import babelParser from '@babel/eslint-parser';

export default [
    js.configs.recommended,
    react.configs.flat['jsx-runtime'],
    importX.configs['flat/recommended'],
    {
        files: ['src/**/*.{js,jsx}'],
        plugins: {
            'react': react,
            'react-hooks': reactHooks,
        },
        rules: {
            'react/jsx-uses-vars': 'error',
            'react/jsx-uses-react': 'error',
            'react/react-in-jsx-scope': 'off',
            'react/prop-types': 'off',
            'import-x/no-duplicates': 'error',
            ...reactHooks.configs.flat['recommended-latest'].rules,
        },
        languageOptions: {
            parser: babelParser,
            parserOptions: {
                requireConfigFile: false,
                babelOptions: {
                    presets: ['@babel/preset-react'],
                },
            },
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