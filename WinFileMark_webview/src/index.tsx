import React from 'react';
import { createRoot } from 'react-dom/client';
import { FluentProvider, webLightTheme } from '@fluentui/react-components';
import App from './App';

const root = createRoot(document.getElementById('root')!);
root.render(
    <FluentProvider theme={webLightTheme}>
        <App />
    </FluentProvider>,
);
