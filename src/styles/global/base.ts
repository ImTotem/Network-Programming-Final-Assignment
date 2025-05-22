import { css } from 'styled-components';

const base = css`
  html {
    font-family:
      'Inter', Roboto, system-ui, 'Segoe UI', Helvetica, Arial, 'Apple Color Emoji',
      'Segoe UI Emoji', 'Segoe UI Symbol', sans-serif;
  }

  body {
    position: relative;
    background: #fafafa;
  }

  button {
    cursor: pointer;
    font-family:
      'Inter', Roboto, system-ui, 'Segoe UI', Helvetica, Arial, 'Apple Color Emoji',
      'Segoe UI Emoji', 'Segoe UI Symbol', sans-serif;
  }

  svg {
    pointer-events: none;
  }
`;

export default base;
