import { BREAK_POINTS } from '@/constants/mediaQueries';

export const xSmall = { minWidth: BREAK_POINTS.XS };
export const small = { minWidth: BREAK_POINTS.S };
export const medium = { minWidth: BREAK_POINTS.M };
export const large = { minWidth: BREAK_POINTS.L };
export const xLarge = { minWidth: BREAK_POINTS.XL };
export const xXLarge = { minWidth: BREAK_POINTS.XXL };
export const notXSmall = { maxWidth: BREAK_POINTS.XS };
export const notSmall = { maxWidth: BREAK_POINTS.S };
export const notMedium = { maxWidth: BREAK_POINTS.M };
export const notLarge = { maxWidth: BREAK_POINTS.L };
export const notXLarge = { maxWidth: BREAK_POINTS.XL };
export const notXXLarge = { maxWidth: BREAK_POINTS.XXL };

export const onXSmall = `@media only screen and (min-width: ${BREAK_POINTS.XS}px)`;
export const onSmall = `@media only screen and (min-width: ${BREAK_POINTS.S}px)`;
export const onMedium = `@media only screen and (min-width: ${BREAK_POINTS.M}px)`;
export const onLarge = `@media only screen and (min-width: ${BREAK_POINTS.L}px)`;
export const onXLarge = `@media only screen and (min-width: ${BREAK_POINTS.XL}px)`;
export const onXXLarge = `@media only screen and (min-width: ${BREAK_POINTS.XXL}px)`;

export const onNotXSmall = `@media only screen and (max-width: ${BREAK_POINTS.XS}px)`;
export const onNotSmall = `@media only screen and (max-width: ${BREAK_POINTS.S}px)`;
export const onNotMedium = `@media only screen and (max-width: ${BREAK_POINTS.M}px)`;
export const onNotLarge = `@media only screen and (max-width: ${BREAK_POINTS.L}px)`;
export const onNotXLarge = `@media only screen and (max-width: ${BREAK_POINTS.XL}px)`;
export const onNotXXLarge = `@media only screen and (max-width: ${BREAK_POINTS.XXL}px)`;
