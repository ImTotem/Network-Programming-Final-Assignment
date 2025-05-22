import { createGlobalStyle } from 'styled-components';

import base from './base';
import { fontFace } from './fonts';
import reset from './reset';

const GlobalStyles = createGlobalStyle`
  ${fontFace}
  ${reset}
  ${base}
`;

export default GlobalStyles;
