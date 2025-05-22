import { createRoot } from 'react-dom/client';

import App from './App.tsx';
import GlobalStyles from './styles/global/index.ts';

createRoot(document.getElementById('root')!).render(
  <>
    <GlobalStyles />
    <App />
  </>
);
