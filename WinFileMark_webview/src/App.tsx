import { HashRouter, Routes, Route } from 'react-router-dom';
import { I18nextProvider } from 'react-i18next';
import FrameLayout from './layouts/FrameLayout'
import Main from './pages/Main';
import About from './pages/About';
import Infobar from './pages/Infobar';
import i18n from './locales';
import "./index.css"

export default function App() {
  return (
    <I18nextProvider i18n={i18n}>
      <HashRouter>
        <Routes>
          <Route path="/" element={<FrameLayout />}>
              <Route path="main" element={<Main />} />
              <Route path="about" element={<About />} />
              <Route path="infobar" element={<Infobar />} />
          </Route>
        </Routes>
      </HashRouter>
    </I18nextProvider>
  );
}
